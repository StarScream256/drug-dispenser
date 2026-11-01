#include <Arduino.h>
#include <Preferences.h>
#include "HX711.h"

#define LOADCELL_DOUT_PIN 19
#define LOADCELL_SCK_PIN 18

HX711 scale;
Preferences prefs;

float calibrationFactor = 1.0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n=== HX711 Load Cell Reader ===");

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

  // Read saved calibration factor from NVS Flash
  prefs.begin("timbangan", true); // Open in read-only mode
  calibrationFactor = prefs.getFloat("faktor", 1.0);
  prefs.end();

  Serial.print("Loaded Calibration Factor: ");
  Serial.println(calibrationFactor);

  scale.set_scale(calibrationFactor);

  Serial.println("Taring scale...");
  scale.tare();
  Serial.println("Scale ready.");
}

void loop() {
  if (scale.is_ready()) {
    float weight = scale.get_units(5);
    Serial.print("Weight: ");
    Serial.print(weight, 2);
    Serial.println(" g");
  } else {
    Serial.println("HX711 not found.");
  }

  delay(500);
}