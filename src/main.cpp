#include <Arduino.h>
#include <WiFi.h>
#include "config.h"
#include "firebase_service.h"

unsigned long lastTestMillis = 0;
const int infraredPin = 23;
int counter = 0;
int irStatus = HIGH;
int lastIrStatus = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 20;

void setup() {
  Serial.begin(115200);
  Serial.println("Starting...");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println("\n[Wi-Fi] Terhubung! IP: " + WiFi.localIP().toString());

  setupFirebase();
}

void loop() {
  loopFirebase();

  int currentReading = digitalRead(infraredPin);

  if (currentReading != lastIrStatus) {
    if ((millis() - lastDebounceTime) > debounceDelay) {
      lastDebounceTime = millis();
      lastIrStatus = currentReading;

      if (currentReading == LOW) {
        counter++;
        Serial.printf("[Sensor IR] Object Passed! Counter: %d\n", counter);
        testInfraredNode(counter);
      }
    }
  }
}