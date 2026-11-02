// #include <Arduino.h>

// #include "test_components/component_tests.h"

// enum TestMode {
//   MODE_IDLE,
//   MODE_BUZZER,
//   MODE_LED,
//   MODE_SERVO,
//   // MODE_LCD,
//   // MODE_IR,
//   MODE_LOADCELL
// };

// TestMode currentMode = MODE_IDLE;

// void printMenu() {
//   Serial.println("\n==========================================");
//   Serial.println("      ESP32 MASTER COMPONENT TESTER       ");
//   Serial.println("==========================================");
//   Serial.println("Send a number to run a test:");
//   Serial.println(" [1] Buzzer Test");
//   Serial.println(" [2] LED Test");
//   Serial.println(" [3] Servo Motor Test");
//   Serial.println(" [4] Load Cell (HX711) Test");
//   Serial.println(" [0] STOP / Reset to Idle");
//   Serial.println("==========================================");
// }

// void setup() {
//   Serial.begin(115200);
//   delay(1000);
//   printMenu();
// }

// void loop() {
//   if (Serial.available() > 0) {
//     const char input = Serial.read();

//     if (input == '\n' || input == '\r') {
//       return;
//     }

//     switch (input) {
//       case '1':
//         currentMode = MODE_BUZZER;
//         buzzerTestSetup();
//         Serial.println(">>> Running: Buzzer Test");
//         break;
//       case '2':
//         currentMode = MODE_LED;
//         ledTestSetup();
//         Serial.println(">>> Running: LED Test");
//         break;
//       case '3':
//         currentMode = MODE_SERVO;
//         servoTestSetup();
//         Serial.println(">>> Running: Servo Motor Test");
//         break;
//       case '4':
//         currentMode = MODE_LOADCELL;
//         loadCellTestSetup();
//         Serial.println(">>> Running: Load Cell Test");
//         break;
//       case '0':
//       default:
//         currentMode = MODE_IDLE;
//         Serial.println(">>> Stopped all tests. Idle state.");
//         printMenu();
//         break;
//     }
//   }

//   switch (currentMode) {
//     case MODE_BUZZER:
//       buzzerTestLoop();
//       break;
//     case MODE_LED:
//       ledTestLoop();
//       break;
//     case MODE_SERVO:
//       servoTestLoop();
//       break;
//     case MODE_LOADCELL:
//       loadCellTestLoop();
//       break;
//     case MODE_IDLE:
//     default:
//       break;
//   }
// }
