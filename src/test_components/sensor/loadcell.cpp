#include <Arduino.h>
#include "HX711.h"

const int LOADCELL_DOUT_PIN = 19;
const int LOADCELL_SCK_PIN = 18;

HX711 scale;

float calibration_factor = 420.0; 

unsigned long lastLoadCellRead = 0;

void loadCellTestSetup() {
  Serial.println("\n[HX711] Initializing Load Cell Test...");
  
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  
  Serial.println("[HX711] Please REMOVE all weights from the scale for taring...");
  delay(2000); 
  
  scale.tare(); // Reset the scale to 0
  Serial.println("[HX711] Taring complete. Zero set.");
  
  scale.set_scale(calibration_factor);
  
  Serial.println("[HX711] Ready! Place a weight on the load cell to test.");
}

void loadCellTestLoop() {
  // Read every 500ms to avoid flooding the Serial Monitor
  if (millis() - lastLoadCellRead >= 500) {
    lastLoadCellRead = millis();
    
    if (scale.is_ready()) {
      // Get the calculated weight units (grams/kilograms depending on calibration_factor)
      float weight = scale.get_units(5); // Average of 5 readings for stability
      
      Serial.print("[HX711] Weight: ");
      Serial.print(weight, 1); // 1 decimal place
      Serial.println(" kg");
    } else {
      Serial.println("[HX711] Error: HX711 not found. Check your wiring!");
    }
  }
}
