#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>
#include "HX711.h"
#include <WiFi.h>

// inisialisasi lcd i2c
LiquidCrystal_I2C lcd(0x27, 16, 2);

const char* ssid = "Wak haji benzema";
const char* password = "supermax";

// konfigurasi 5 servo
const int NUM_SERVOS = 5;
Servo servos[NUM_SERVOS];
const int servoPins[NUM_SERVOS] = {13, 12, 14, 27, 26};

// posisi sudut servo
const int POS_CLOSED = 0;   // tabung tertutup
const int POS_OPEN = 90;    // tabung tertutup

// infrared pin
const int IR_PIN = 34;
volatile int drugCount = 0;

// konfigurasi load cell
const int LOADCELL_DOUT_PIN = 4;
const int LOADCELL_SCK_PIN = 16;
HX711 scale;

// ambang batas berat (dalam gram).
const float WEIGHT_THRESHOLD = 2.0; 

// simulasi target obat
int targetDrugCount[NUM_SERVOS] = {3, 2, 4, 1, 5}; 

// Interrupt Service Routine (ISR) sensor IR
void countDrugISR() {
  drugCount++;
}

void connectToWiFi() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Konek Wi-Fi...");
  
  WiFi.begin(ssid, password);
  
  int timeout = 0;
  while (WiFi.status() != WL_CONNECTED && timeout < 20) {
    delay(500);
    lcd.setCursor(timeout % 16, 1);
    lcd.print(".");
    timeout++;
  }

  lcd.clear();
  if (WiFi.status() == WL_CONNECTED) {
    lcd.setCursor(0, 0);
    lcd.print("WiFi Terhubung!");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP()); // Menampilkan IP lokal
  } else {
    lcd.setCursor(0, 0);
    lcd.print("WiFi Gagal!");
    lcd.setCursor(0, 1);
    lcd.print("Lanjut Offline");
  }
  delay(2000);
}

void waitForTrayEmptiedAndDelay();
void updateLCDCount(int current, int target);
void dispenseAllDrugs();


void setup() {
  // Inisialisasi LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Dispenser Obat");
  lcd.setCursor(0, 1);
  lcd.print("Inisialisasi...");

  // coba connect ke wifi
  connectToWiFi();

  // Inisialisasi Load Cell
  // scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  // Masukkan faktor kalibrasi Load Cell Anda di sini (contoh: 420.0)
  // scale.set_scale(420.0); 
  // scale.tare(); // Reset / Zero-ing berat nampan kosong di awal

  // Inisialisasi Servo
  for (int i = 0; i < NUM_SERVOS; i++) {
    servos[i].attach(servoPins[i]);
    servos[i].write(POS_CLOSED);
  }

  // Inisialisasi Sensor IR
  pinMode(IR_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(IR_PIN), countDrugISR, FALLING);

  delay(2000);

  // Mulai Proses Dispensing
  dispenseAllDrugs();
}

void loop() {
  // Loop utama kosong karena proses berjalan di setup()
}

void dispenseAllDrugs() {
  for (int i = 0; i < NUM_SERVOS; i++) {
    
    // PERIKSA LOAD CELL SEBELUM TABUNG BERIKUTNYA (Mulai dari Obat B/indeks 1 ke atas)
    if (i > 0) {
      waitForTrayEmptiedAndDelay();
    }

    int target = targetDrugCount[i];
    drugCount = 0; // Reset hitungan obat untuk tabung aktif

    // Tampilkan status di LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Obat ");
    lcd.print((char)('A' + i));
    lcd.print(" Target:");
    lcd.print(target);

    // Buka dan tutup servo secara berkala sampai target tercapai
    while (drugCount < target) {
      updateLCDCount(drugCount, target);

      // Buka sekat obat
      servos[i].write(POS_OPEN);
      delay(500); 

      // Tutup kembali sekat
      servos[i].write(POS_CLOSED);
      delay(800); 
    }

    updateLCDCount(drugCount, target);
    
    // Status tabung selesai
    lcd.setCursor(0, 0);
    lcd.print("Obat ");
    lcd.print((char)('A' + i));
    lcd.print(" Selesai!");
    delay(1000);
  }

  // Status Akhir
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Semua Selesai!");
  lcd.setCursor(0, 1);
  lcd.print("Silakan Ambil");
}

// --- Fungsi Menunggu Nampan Kosong & Jeda 5 Detik ---
void waitForTrayEmptiedAndDelay() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Ambil Obat Anda!");

  // Tunggu hingga obat diambil (Load Cell membaca berat <= WEIGHT_THRESHOLD)
  while (scale.get_units(5) > WEIGHT_THRESHOLD) {
    lcd.setCursor(0, 1);
    lcd.print("Nampan Terisi   ");
    delay(200);
  }

  // Nampan sudah kosong, lakukan Countdown Jeda 5 Detik
  for (int seconds = 5; seconds > 0; seconds--) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Nampan Kosong!");
    lcd.setCursor(0, 1);
    lcd.print("Lanjut dlm ");
    lcd.print(seconds);
    lcd.print(" dtk");
    delay(1000);
  }
}

// Fungsi Update Tampilan Jumlah Obat di LCD
void updateLCDCount(int current, int target) {
  lcd.setCursor(0, 1);
  lcd.print("Keluar: ");
  lcd.print(current);
  lcd.print("/");
  lcd.print(target);
  lcd.print("   ");
}