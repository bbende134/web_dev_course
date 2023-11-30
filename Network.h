#if defined(ESP32) || defined(ARDUINO_RASPBERRY_PI_PICO_W)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#elif __has_include(<WiFiNINA.h>)
#include <WiFiNINA.h>
#elif __has_include(<WiFi101.h>)
#include <WiFi101.h>
#elif __has_include(<WiFiS3.h>)
#include <WiFiS3.h>
#endif


#include <Firebase_ESP_Client.h>
#include <ArduinoJson.h>

class Network {
private:
  FirebaseData fbdo;
  FirebaseAuth auth;
  FirebaseConfig config;

public:
  Network();
  void initWiFi();
  void firebaseInit();
  int postWebhooks(String value1);
  bool writeTemperatureData(double temp, String ts);
  bool readPost();
  bool readIsHome();
  bool firebaseReady();
};