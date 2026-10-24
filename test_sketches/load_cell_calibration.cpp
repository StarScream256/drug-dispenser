#include "HX711.h"
#include <Preferences.h> // Library bawaan ESP32 untuk simpan data non-volatile

// Konfigurasi Pin
const int LOADCELL_DOUT_PIN = 5;
const int LOADCELL_SCK_PIN = 18;

HX711 scale;
Preferences prefs;

// Tentukan berat sensor IR Anda (Sesuaikan di sini, misal diasumsikan 4.0 gram)
float berat_referensi = 4.0; 
float faktor_kalibrasi_saved = 1.0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n=== TIMBANGAN HX711 DENGAN PENYIMPANAN ESP32 ===");

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  prefs.begin("timbangan", false); 
  
  // Jika belum pernah disimpan, nilai default-nya adalah 1.0
  faktor_kalibrasi_saved = prefs.getFloat("faktor", 1.0);
  
  Serial.print("Nilai kalibrasi yang tersimpan di memori: ");
  Serial.println(faktor_kalibrasi_saved);

  scale.set_scale(faktor_kalibrasi_saved);
  
  // tare (nol-kan timbangan) saat start
  Serial.println("Mengosongkan timbangan (Tare)...");
  scale.tare();
  Serial.println("Timbangan siap digunakan.");
  
  Serial.println("\n--- MENU ---");
  Serial.println("Ketik 'K' lalu Enter di Serial Monitor jika ingin melakukan KALIBRASI ULANG.");
  Serial.println("------------\n");
}

void jalankanKalibrasiOtomatis();

void loop() {
  // Tampilkan pembacaan berat secara kontinu
  if (scale.is_ready()) {
    float berat = scale.get_units(5); // Ambil rata-rata 5 kali pembacaan
    Serial.print("Berat Saat Ini: ");
    Serial.print(berat, 2); // Menampilkan 2 angka di belakang koma
    Serial.println(" gram");
  } else {
    Serial.println("HX711 tidak ditemukan.");
  }
  
  // Cek apakah user mengetik perintah kalibrasi di Serial Monitor
  if (Serial.available() > 0) {
    char perintah = Serial.read();
    if (perintah == 'K' || perintah == 'k') {
      jalankanKalibrasiOtomatis();
    }
  }
  delay(500);
}

// Fungsi Khusus Kalibrasi Baru dan Menyimpannya ke Memori Internal
void jalankanKalibrasiOtomatis() {
  Serial.println("\n>>> MEMULAI PROSES KALIBRASI ULANG <<<");
  scale.set_scale(); // Reset skala ke default dulu
  
  Serial.println("1. Kosongkan load cell sepenuhnya.");
  Serial.println("Menghitung titik nol (Tare) dalam 3 detik...");
  delay(3000);
  scale.tare();
  Serial.println("Tare Selesai.");
  
  Serial.print("2. SEKARANG letakkan Sensor IR (Beban: ");
  Serial.print(berat_referensi);
  Serial.println(" gram) di atas load cell.");
  
  // Waktu tunggu 10 detik untuk meletakkan benda
  for(int i = 10; i > 0; i--) {
    Serial.print("Membaca sensor dalam: ");
    Serial.println(i);
    delay(1000);
  }
  
  // Ambil data mentah pembacaan sensor
  long nilai_mentah = scale.get_units(10);
  
  // Rumus mencari faktor kalibrasi baru
  float faktor_baru = (float)nilai_mentah / berat_referensi;
  
  // Simpan nilai baru ke dalam flash memory ESP32
  prefs.putFloat("faktor", faktor_baru);
  
  Serial.println("\n>>> KALIBRASI BERHASIL DAN DISIMPAN! <<<");
  Serial.print("Faktor Baru: ");
  Serial.println(faktor_baru);
  
  // Terapkan langsung ke sistem aktif
  scale.set_scale(faktor_baru);
  
  Serial.println("Silakan lanjut menimbang. Sistem akan selalu mengingat nilai ini.");
  Serial.println("---------------------------------------------------\n");
}