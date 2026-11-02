#include <Arduino.h>

#define LED_PIN 26

void ledTestSetup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  Serial.println("LED test initialized.");
}

void ledTestLoop() {
  Serial.println("LED ON");
  digitalWrite(LED_PIN, HIGH);
  delay(2000);

  Serial.println("LED OFF");
  digitalWrite(LED_PIN, LOW);
  delay(2000);
}