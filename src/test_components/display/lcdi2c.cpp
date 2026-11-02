#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Parameters: (I2C_ADDRESS, COLUMNS, ROWS)
LiquidCrystal_I2C lcd(0x27, 16, 2);

void lcdI2cTestSetup() {
  Serial.begin(115200);

  Wire.begin();

  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0); // (col, row)
  lcd.print("LCD Test Ready");
  
  Serial.println("LCD I2C test initialized.");
  delay(2000);
  lcd.clear();
}

void lcdI2cTestLoop() {
  // Line 1: Static label
  lcd.setCursor(0, 0);
  lcd.print("Status: Active");

  // Line 2: Dynamic counter
  static int counter = 0; // static: initilized once
  lcd.setCursor(0, 1);
  lcd.print("Count: ");
  lcd.print(counter++);

  Serial.print("LCD updated, count: ");
  Serial.println(counter);

  delay(1000);
}