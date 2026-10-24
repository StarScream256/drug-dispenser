#include "firebase_service.h"

UserAuth user_auth(FIREBASE_API_KEY, FIREBASE_USER_EMAIL, FIREBASE_USER_PASSWORD);
FirebaseApp app;
WiFiClientSecure ssl_client;
using AsyncClient = AsyncClientClass;
AsyncClient async_client(ssl_client);
RealtimeDatabase Database;
AsyncResult dbResult;

void setupFirebase() {
  // SSL Settings (Optional/Dev mode)
  ssl_client.setInsecure();
  ssl_client.setHandshakeTimeout(5);

  Serial.println("[Firebase] Memulai Inisialisasi Auth...");
  initializeApp(async_client, app, getAuth(user_auth), processData, "🔐 authTask");

  // bind Realtime Database instance to the app
  app.getApp<RealtimeDatabase>(Database);
  Database.url(FIREBASE_URL);
}

void loopFirebase() {
  app.loop();
  processData(dbResult);
}

void testInfraredNode(int count) {
  if (!app.ready()) {
    Serial.println("[Firebase] Not ready yet ...");
    return;
  }

  // send the count and infrared status to Firebase Realtime Database
  Serial.printf("[Firebase] Mengirim infrared_count: %d ...\n", count);
  Database.set<int>(async_client, "/test/infrared_count", count, dbResult);
}

// implementasi Handler untuk memproses Event/Error/Data
void processData(AsyncResult &aResult) {
  if (!aResult.isResult())
    return;

  if (aResult.isEvent())
    Firebase.printf("Event task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.eventLog().message().c_str(), aResult.eventLog().code());

  if (aResult.isDebug())
    Firebase.printf("Debug task: %s, msg: %s\n", aResult.uid().c_str(), aResult.debug().c_str());

  if (aResult.isError())
    Firebase.printf("Error task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.error().message().c_str(), aResult.error().code());

  if (aResult.available())
    Firebase.printf("Task: %s, payload: %s\n", aResult.uid().c_str(), aResult.c_str());
}