#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>
#include <Preferences.h>
#include "HX711.h"

#define BUZZER_PIN 25
#define LED_PIN 12
#define SERVO_PIN 5
#define IR_SENSOR_PIN 23
#define LOADCELL_DOUT 19
#define LOADCELL_SCK 18

LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo myServo;
HX711 scale;
Preferences prefs;

// Test Modes
enum TestMode {
  MODE_IDLE,
  MODE_BUZZER,
  MODE_LED,
  MODE_SERVO,
  MODE_LCD,
  MODE_IR,
  MODE_LOADCELL
};

TestMode currentMode = MODE_IDLE;
unsigned long lastActionTime = 0;

void printMenu() {
  Serial.println("\n==========================================");
  Serial.println("      ESP32 MASTER COMPONENT TESTER       ");
  Serial.println("==========================================");
  Serial.println("Send a number to run a test:");
  Serial.println(" [1] Buzzer Test");
  Serial.println(" [2] LED Test");
  Serial.println(" [3] Servo Motor Test");
  Serial.println(" [4] LCD I2C Test");
  Serial.println(" [5] IR Obstacle Sensor Test");
  Serial.println(" [6] Load Cell (HX711) Test");
  Serial.println(" [0] STOP / Reset to Idle");
  Serial.println("==========================================");
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(IR_SENSOR_PIN, INPUT);

  Wire.begin();
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Master Test Ready");

  myServo.attach(SERVO_PIN, 500, 2400);
  
  scale.begin(LOADCELL_DOUT, LOADCELL_SCK);
  prefs.begin("timbangan", true);
  float calFactor = prefs.getFloat("faktor", 1.0);
  prefs.end();
  scale.set_scale(calFactor);
  scale.tare();

  printMenu();
}

void resetOutputs() {
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_PIN, LOW);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Mode: Idle");
}

void loop() {
  if (Serial.available() > 0) {
    char input = Serial.read();
    
    if (input == '\n' || input == '\r') return;

    resetOutputs();

    switch (input) {
      case '1':
        currentMode = MODE_BUZZER;
        Serial.println(">>> Running: Buzzer Test");
        break;
      case '2':
        currentMode = MODE_LED;
        Serial.println(">>> Running: LED Test");
        break;
      case '3':
        currentMode = MODE_SERVO;
        Serial.println(">>> Running: Servo Motor Test");
        break;
      case '4':
        currentMode = MODE_LCD;
        Serial.println(">>> Running: LCD I2C Test");
        break;
      case '5':
        currentMode = MODE_IR;
        Serial.println(">>> Running: IR Obstacle Sensor Test");
        break;
      case '6':
        currentMode = MODE_LOADCELL;
        Serial.println(">>> Running: Load Cell Test");
        break;
      case '0':
      default:
        currentMode = MODE_IDLE;
        Serial.println(">>> Stopped all tests. Idle state.");
        printMenu();
        break;
    }
  }

  unsigned long now = millis();

  switch (currentMode) {
    case MODE_BUZZER:
      if (now - lastActionTime >= 500) {
        lastActionTime = now;
        digitalWrite(BUZZER_PIN, !digitalRead(BUZZER_PIN));
      }
      break;

    case MODE_LED:
      if (now - lastActionTime >= 500) {
        lastActionTime = now;
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
      }
      break;

    case MODE_SERVO:
      if (now - lastActionTime >= 1000) {
        lastActionTime = now;
        static int angle = 0;
        angle = (angle == 0) ? 90 : (angle == 90) ? 180 : 0;
        myServo.write(angle);
        Serial.print("Servo Position: ");
        Serial.println(angle);
      }
      break;

    case MODE_LCD:
      if (now - lastActionTime >= 1000) {
        lastActionTime = now;
        static int count = 0;
        lcd.setCursor(0, 0);
        lcd.print("LCD Test Active ");
        lcd.setCursor(0, 1);
        lcd.print("Counter: ");
        lcd.print(count++);
      }
      break;

    case MODE_IR:
      if (now - lastActionTime >= 200) {
        lastActionTime = now;
        int state = digitalRead(IR_SENSOR_PIN);
        Serial.println(state == LOW ? "Obstacle Detected!" : "Path Clear");
      }
      break;

    case MODE_LOADCELL:
      if (now - lastActionTime >= 500) {
        lastActionTime = now;
        if (scale.is_ready()) {
          float weight = scale.get_units(3);
          Serial.print("Weight: ");
          Serial.print(weight, 2);
          Serial.println(" g");
        } else {
          Serial.println("HX711 connection error.");
        }
      }
      break;

    case MODE_IDLE:
    default:
      break;
  }
}