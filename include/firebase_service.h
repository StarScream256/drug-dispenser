#ifndef FIREBASE_SERVICE_H
#define FIREBASE_SERVICE_H

// Step 1: Macro penanda fitur yang digunakan
#define ENABLE_USER_AUTH
#define ENABLE_DATABASE

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <FirebaseClient.h>
#include "config.h"

// Step 2: Deklarasi fungsi callback penanganan data/event
void processData(AsyncResult &aResult);

// Fungsi antarmuka utama
void setupFirebase();
void loopFirebase();
void testInfraredNode(int count);

#endif