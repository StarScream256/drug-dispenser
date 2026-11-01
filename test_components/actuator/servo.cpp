#include <Arduino.h>
#include <ESP32Servo.h>

#define SERVO_PIN 5

Servo myServo;

void setup() {
  Serial.begin(115200);

  myServo.attach(SERVO_PIN);
  
  Serial.println("Servo test initialized.");
}

void loop() {
  Serial.println("Moving to 0 degrees");
  myServo.write(0);
  delay(1000);

  Serial.println("Moving to 90 degrees");
  myServo.write(90);
  delay(1000);

  Serial.println("Moving to 180 degrees");
  myServo.write(180);
  delay(1000);
}