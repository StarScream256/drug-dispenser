#include <Arduino.h>
#include <WiFi.h>
#include "config.h"
#include "firebase_service.h"

unsigned long lastTestMillis = 0;
int counter = 0;

void setup() {
  Serial.begin(115200);

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
  Serial.println("[Loop] Looping...");
  if (millis() - lastTestMillis > 5000) {
    lastTestMillis = millis();
    counter++;
    testInfraredNode(counter);
  }
}