#include <vector>
#include <NTPClient.h>
#include <WiFiUdp.h>
// #include <chrono>
#include "Network.h"
#include "addons/TokenHelper.h"
#include <HTTPClient.h>
#include <ESPDateTime.h>

// Firebase data
#define API_KEY "AIzaSyB00o4-ticI3Bt7cP54shxEFBteqGVwyII";
#define FIREBASE_PROJECT_ID "fgdh-74592"
#define USER_EMAIL "admin@admin.com"
#define USER_PASSWORD "admin1234"

// Wifi data
#define WIFI_SSID "VOL 23"
#define WIFI_PASSWORD "135792468"

static Network* instance = NULL;

// IFTTT data
String eventNAME = "new_mail";
String webhooksKEY = "bk-rNjIlmxawG-VkvLWg9K";
const int httpsPort = 443;
String url = "https://maker.ifttt.com/trigger/" + eventNAME + "/with/key/" + webhooksKEY;


Network::Network() {
  instance = this;
}

void Network::initWiFi() {
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);

#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
  multi.addAP(WIFI_SSID, WIFI_PASSWORD);
  multi.run();
#else
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
#endif

  Serial.print("Connecting to Wi-Fi");
  unsigned long ms = millis();
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
    if (millis() - ms > 10000)
      break;
#endif
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
}

bool Network::writeTemperatureData(double temp, String ts) {  // TODO: implement for the new DB

  FirebaseJson content;
  ts.remove(ts.length() - 5, 5);
  ts += "Z";

  String documentPath = "temperature/actual_temp_" + ts;
  content.set("fields/type/stringValue/", "actual_temperature");
  content.set("fields/value/doubleValue/", temp);
  content.set("fields/ts/timestampValue/", ts);

  if (Firebase.Firestore.createDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str(), content.raw())) {
    Serial.println("ok write actual temperature");
    return 1;
  } else {
    Serial.println(fbdo.errorReason());
    return 0;
  }
}

void Network::firebaseInit() {
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  config.token_status_callback = tokenStatusCallback;  // see addons/TokenHelper.h
  Firebase.reconnectNetwork(true);
  fbdo.setBSSLBufferSize(4096, 1024);

  // Limit the size of response payload to be collected in FirebaseData
  fbdo.setResponseSize(2048);

  Firebase.begin(&config, &auth);
}


int Network::postWebhooks(String value1) {

  if (WiFi.status() == WL_CONNECTED) {  //Check WiFi connection status
    HTTPClient http;

    String url_out = url + "?value1=" + value1;
    Serial.print("url: ");
    Serial.println(url_out);

    http.begin(url_out);                   //Specify destination for HTTP request
    int httpResponseCode = http.POST("");  //Send the actual POST request

    if (httpResponseCode > 0) {

      String response = http.getString();  //Get the response to the request

      Serial.println(httpResponseCode);  //Print return code
      Serial.println(response);
      return httpResponseCode;

    } else {

      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
      return httpResponseCode;
    }

    http.end();  //Free resources

  } else {
    Serial.println("Error in WiFi connection");
    return 0;
  }
}

bool Network::firebaseReady() {
  return Firebase.ready();
}