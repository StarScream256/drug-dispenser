#include <Arduino.h>

#define IR_SENSOR_PIN 19

void setup() {
  Serial.begin(115200);
  Serial.println("IR Obstacle Sensor test initialized.");
}

void loop() {
  int sensorState = digitalRead(IR_SENSOR_PIN);

  if (sensorState == LOW) {
    Serial.println("Obstacle Detected!");
  } else {
    Serial.println("Path Clear");
  }

  delay(200);
}