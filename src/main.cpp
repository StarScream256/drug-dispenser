#include <ESP32Servo.h>
#include <HX711.h>

// =========================================================================
// DEFINISI PIN ESP32
// =========================================================================
#define IR_PIN_1        34  // Sensor IR Tabung 1 (GPIO 34)
#define IR_PIN_2        35  // Sensor IR Tabung 2 (GPIO 35)
#define IR_PIN_3        36  // Sensor IR Tabung 3 (GPIO 36 / VP)
#define IR_PIN_4        39  // Sensor IR Tabung 4 (GPIO 39 / VN)

#define HX711_DT_PIN    19  // Data Load Cell (GPIO 19)
#define HX711_SCK_PIN   18  // Clock Load Cell (GPIO 18)

#define SERVO_PIN_1     4   // Servo Katup Tabung 1 (GPIO 4)
#define SERVO_PIN_2     21  // Servo Katup Tabung 2 (GPIO 21)
#define SERVO_PIN_3     22  // Servo Katup Tabung 3 (GPIO 22)
#define SERVO_PIN_4     23  // Servo Katup Tabung 4 (GPIO 23)

// LED Notifikasi & Pengingat Stok
#define LED_MAIN_DONE   26  // LED Utama: Obat Siap / Peracikan Selesai (GPIO 26)
#define LED_STOCK_1     14  // LED Stok Kritis Tabung 1 (GPIO 14)
#define LED_STOCK_2     16  // LED Stok Kritis Tabung 2 (GPIO 16)
#define LED_STOCK_3     17  // LED Stok Kritis Tabung 3 (GPIO 17)
#define LED_STOCK_4     13  // LED Stok Kritis Tabung 4 (GPIO 13)

#define BUZZER_PIN      25  // Alarm Audio Buzzer (GPIO 25)

// =========================================================================
// DEKLARASI OBJEK & VARIABEL GLOBAL
// =========================================================================
Servo servoTabung[4];
HX711 loadCell;

const int servoPins[4] = {SERVO_PIN_1, SERVO_PIN_2, SERVO_PIN_3, SERVO_PIN_4};
const int irPins[4]    = {IR_PIN_1, IR_PIN_2, IR_PIN_3, IR_PIN_4};
const int ledStock[4]  = {LED_STOCK_1, LED_STOCK_2, LED_STOCK_3, LED_STOCK_4};

// Variabel Pencacah Dosis Obat (Volatile untuk ISR Interrupt)
volatile int pillCount[4] = {0, 0, 0, 0};
unsigned long lastDebounceTime[4] = {0, 0, 0, 0};
const unsigned long DEBOUNCE_DELAY = 150; // ms

// Parameter Resep & Manajemen Stok Chamber
int targetDosis[4]   = {3, 2, 4, 1};  // Target Dosis Resep (Tabung 1=3, Tabung 2=2, Tabung 3=4, Tabung 4=1)
int virtualStock[4]  = {12, 15, 8, 20}; // Stok Awal Obat di Setiap Chamber
const int MIN_STOCK_THRESHOLD = 10;   // Ambang Batas Stok Menipis (< 10)

// Status Sistem
bool isDispensing = false;
bool servoClosed[4] = {true, true, true, true};
float calibrationFactor = 420.0; // Faktor Kalibrasi Load Cell

// =========================================================================
// INTERRUPT SERVICE ROUTINES (ISR) PENCACAH SENSOR IR
// =========================================================================
void IRAM_ATTR isrIR1() {
  unsigned long now = millis();
  if (now - lastDebounceTime[0] > DEBOUNCE_DELAY) {
    pillCount[0]++;
    lastDebounceTime[0] = now;
  }
}

void IRAM_ATTR isrIR2() {
  unsigned long now = millis();
  if (now - lastDebounceTime[1] > DEBOUNCE_DELAY) {
    pillCount[1]++;
    lastDebounceTime[1] = now;
  }
}

void IRAM_ATTR isrIR3() {
  unsigned long now = millis();
  if (now - lastDebounceTime[2] > DEBOUNCE_DELAY) {
    pillCount[2]++;
    lastDebounceTime[2] = now;
  }
}

void IRAM_ATTR isrIR4() {
  unsigned long now = millis();
  if (now - lastDebounceTime[3] > DEBOUNCE_DELAY) {
    pillCount[3]++;
    lastDebounceTime[3] = now;
  }
}

// =========================================================================
// SETUP AWAL SISTEM
// =========================================================================
void setup() {
  Serial.begin(115200);
  Serial.println(F("\n=================================================="));
  Serial.println(F(" AUTOMATED DISPENSING CABINET (ADC) - PARALEL MODE"));
  Serial.println(F("=================================================="));

  // Inisialisasi Pin Output LED & Buzzer
  pinMode(LED_MAIN_DONE, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(LED_MAIN_DONE, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  for (int i = 0; i < 4; i++) {
    pinMode(ledStock[i], OUTPUT);
    digitalWrite(ledStock[i], LOW);
  }

  // Inisialisasi 4 Servo (Posisi Katup Tertutup = 0 Derajat)
  for (int i = 0; i < 4; i++) {
    servoTabung[i].attach(servoPins[i]);
    servoTabung[i].write(0);
    servoClosed[i] = true;
  }

  // Inisialisasi Pin Input Sensor IR + Interrupt FALLING
  pinMode(IR_PIN_1, INPUT);
  pinMode(IR_PIN_2, INPUT);
  pinMode(IR_PIN_3, INPUT);
  pinMode(IR_PIN_4, INPUT);

  attachInterrupt(digitalPinToInterrupt(IR_PIN_1), isrIR1, FALLING);
  attachInterrupt(digitalPinToInterrupt(IR_PIN_2), isrIR2, FALLING);
  attachInterrupt(digitalPinToInterrupt(IR_PIN_3), isrIR3, FALLING);
  attachInterrupt(digitalPinToInterrupt(IR_PIN_4), isrIR4, FALLING);

  // Inisialisasi Load Cell HX711
  loadCell.begin(HX711_DT_PIN, HX711_SCK_PIN);
  loadCell.set_scale(calibrationFactor);
  loadCell.tare(); // Zero-reset nampan kosong

  // Pengecekan Stok Awal Setiap Chamber
  checkStockStatus();
  printMenuHelp();
}

// =========================================================================
// LOOP UTAMA PROGRAM
// =========================================================================
void loop() {
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    if (cmd == 's' || cmd == 'S') {
      jalankanPeracikanSimultan();
    } else if (cmd == 'r' || cmd == 'R') {
      resetSystem();
    }
  }
}

// =========================================================================
// FUNGSI PERACIKAN OBAT SIMULTAN (4 KATUP SERVO BERJALAN BERSAMAAN)
// =========================================================================
void jalankanPeracikanSimultan() {
  if (isDispensing) return;
  isDispensing = true;
  digitalWrite(LED_MAIN_DONE, LOW); // Matikan LED Selesai saat peracikan dimulai

  Serial.println(F("\n[INFO] Memulai Peracikan Obat Simultan (4 Katup)..."));

  // 1. Validasi Stok Semua Chamber Sebelum Membuka Katup
  for (int i = 0; i < 4; i++) {
    if (targetDosis[i] > 0 && virtualStock[i] < targetDosis[i]) {
      Serial.print(F("[ERROR] Stok Chamber ")); Serial.print(i + 1);
      Serial.print(F(" Kurang! Stok: ")); Serial.print(virtualStock[i]);
      Serial.print(F(" | Dibutuhkan: ")); Serial.println(targetDosis[i]);
      triggerErrorAlert();
      isDispensing = false;
      return;
    }
  }

  // 2. Tampilkan Informasi Stok Awal & Target Dosis
  for (int i = 0; i < 4; i++) {
    pillCount[i] = 0;
    Serial.print(F(" -> Chamber ")); Serial.print(i + 1);
    Serial.print(F(": Target = ")); Serial.print(targetDosis[i]);
    Serial.print(F(" pil | Stok Sisa: ")); Serial.println(virtualStock[i]);
  }

  // 3. Buka SEMUA Katup Servo yang Memiliki Target Dosis secara Bersamaan
  Serial.println(F("\n[AKTUASI] Membuka Seluruh Katup Servo Aktif (Sudut 90°)..."));
  for (int i = 0; i < 4; i++) {
    if (targetDosis[i] > 0) {
      servoTabung[i].write(90);
      servoClosed[i] = false;
    }
  }

  // 4. Monitoring Pencacahan IR Simultan (Loop Paralel)
  int lastLoggedCount[4] = {-1, -1, -1, -1};
  bool allChambersFinished = false;

  while (!allChambersFinished) {
    allChambersFinished = true;

    for (int i = 0; i < 4; i++) {
      if (targetDosis[i] > 0) {
        // Tampilkan update log saat ada obat jatuh
        if (pillCount[i] != lastLoggedCount[i]) {
          lastLoggedCount[i] = pillCount[i];
          Serial.print(F("   [Chamber ")); Serial.print(i + 1);
          Serial.print(F("] Pil Jatuh! Progress: "));
          Serial.print(pillCount[i]); Serial.print(F(" / "));
          Serial.println(targetDosis[i]);
        }

        // Tutup katup servo chamber tertentu jika target dosisnya sudah tercapai
        if (pillCount[i] >= targetDosis[i] && !servoClosed[i]) {
          servoTabung[i].write(0); // Tutup sekat katup
          servoClosed[i] = true;
          Serial.print(F("   [SUCCESS] Chamber ")); Serial.print(i + 1);
          Serial.println(F(" Target Terpenuhi -> Katup Ditutup!"));
        }

        // Jika masih ada chamber yang belum selesai, loop berlanjut
        if (pillCount[i] < targetDosis[i]) {
          allChambersFinished = false;
        }
      }
    }
    delay(20);
  }

  // 5. Kurangi Stok Virtual Setelah Peracikan Berhasil
  for (int i = 0; i < 4; i++) {
    virtualStock[i] -= targetDosis[i];
  }

  // 6. Verifikasi Ganda Berat Akumulatif (Load Cell)
  Serial.println(F("\n[VERIFIKASI] Menimbang Bobot Akumulatif Nampan 4 Bilik..."));
  delay(500);
  float finalWeight = loadCell.get_units(5);
  Serial.print(F("[LOAD CELL] Total Bobot Terukur: "));
  Serial.print(finalWeight, 2); Serial.println(F(" gram"));

  // 7. Update Indikator Stok Kritis (< 10)
  checkStockStatus();

  // 8. Pemicuan Reminder (Nyalakan LED Pin 26 & Buzzer Reminder Selesai)
  digitalWrite(LED_MAIN_DONE, HIGH); // LED Pin 26 ON (Obat Siap)
  triggerSuccessReminderBuzzer();

  Serial.println(F("=================================================="));
  Serial.println(F(" STATUS: PERACIKAN SIMULTAN SELESAI & OBAT SIAP! "));
  Serial.println(F("==================================================\n"));
  isDispensing = false;
}

// =========================================================================
// FUNGSI MANAJEMEN STOK & ALERT REMINDER
// =========================================================================
void checkStockStatus() {
  Serial.println(F("\n--- STATUS STOK SETIAP CHAMBER ---"));
  for (int i = 0; i < 4; i++) {
    Serial.print(F(" Chamber ")); Serial.print(i + 1);
    Serial.print(F(": Sisa Stok = ")); Serial.print(virtualStock[i]);

    // Jika stok kurang dari 10, nyalakan LED Peringatan Stok Tabung i
    if (virtualStock[i] < MIN_STOCK_THRESHOLD) {
      digitalWrite(ledStock[i], HIGH);
      Serial.println(F(" -> [STOK KRITIS < 10] (LED ON)"));
    } else {
      digitalWrite(ledStock[i], LOW);
      Serial.println(F(" -> [STOK AMAN] (LED OFF)"));
    }
  }
  Serial.println(F("----------------------------------\n"));
}

void triggerSuccessReminderBuzzer() {
  // Pola alarm pengingat audio bahwa obat sudah siap diambil
  for (int i = 0; i < 4; i++) {
    tone(BUZZER_PIN, 1200); // Frekuensi 1.2 kHz
    delay(120);
    noTone(BUZZER_PIN);
    delay(80);
  }
}

void triggerErrorAlert() {
  for (int i = 0; i < 2; i++) {
    tone(BUZZER_PIN, 400); // 400 Hz Alarm Error
    delay(400);
    noTone(BUZZER_PIN);
    delay(200);
  }
}

void resetSystem() {
  isDispensing = false;
  digitalWrite(LED_MAIN_DONE, LOW);
  for (int i = 0; i < 4; i++) {
    pillCount[i] = 0;
    servoTabung[i].write(0);
    servoClosed[i] = true;
  }
  checkStockStatus();
  Serial.println(F("[SYSTEM] Reset Berhasil. Siap untuk Peracikan Baru."));
}

void printMenuHelp() {
  Serial.println(F("\n[PETUNJUK SIMULASI WOKWI]:"));
  Serial.println(F(" 1. Ketik 's' lalu Enter pada Serial Monitor untuk memicu peracikan simultan."));
  Serial.println(F(" 2. Picu IR Sensor/Tombol TRIG pada WOKWI untuk mensimulasikan jatuhnya pil."));
  Serial.println(F(" 3. Ketik 'r' lalu Enter untuk mereset status peracikan.\n"));
}
