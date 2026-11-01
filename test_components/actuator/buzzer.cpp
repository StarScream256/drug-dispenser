#include <Arduino.h>

#define BUZZER_PIN 25

void setup() {
  Serial.begin(115200);
  pinMode(BUZZER_PIN, OUTPUT);
  Serial.println("Buzzer test initialized.");
}

void loop() {
  Serial.println("Buzzer ON");
  digitalWrite(BUZZER_PIN, HIGH);
  delay(2000);

  Serial.println("Buzzer OFF");
  digitalWrite(BUZZER_PIN, LOW);
  delay(2000);
}