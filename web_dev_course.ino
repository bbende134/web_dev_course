#include <Arduino.h>
#include <math.h>
#include <ESPDateTime.h>
#include "Adafruit_MCP9808.h"
#include "Network.h"

#define TRIG_PIN 23  // GPIO pin connected to the Trig pin of the sensor
#define ECHO_PIN 22  // GPIO pin connected to the Echo pin of the sensor

// I2C communication with MCP9809
#define SDA_0 18
#define SCL_0 19

unsigned long current_millis_distance = 0;
unsigned long time_interval_distane = 50;  // set time intervall for distance measurement
unsigned long previous_millis_distance = 0;
long previous_distance = -1;   // Initialize to a value that is unlikely to occur
long distance_interval = 100;  // Set the distance interval in cm

unsigned long current_millis_temp = 0;
unsigned long time_interval_temp = 10000;  // set time intervall for distance measurement
unsigned long previous_millis_temp = 0;

String ts;
Network* network;

Adafruit_MCP9808 tempsensor = Adafruit_MCP9808();
TwoWire wires = (TwoWire(0));

void setup() {
  Serial.begin(115200);

  // Init of network
  initNetwork();

  // set up datetime
  setupDateTime();

  // init firebase
  network->firebaseInit();

  // init of distance sensor
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // init of temperature sensor
  wires.setPins(SDA_0, SCL_0);
  if (!tempsensor.begin(0x18, &wires)) {
    Serial.println("Couldn't find MCP9808! Check your connections and verify the address is correct.");
    while (1)
      ;
  }

  Serial.println("Found MCP9808!");
  tempsensor.setResolution(3);
}

void loop() {
  current_millis_distance = millis();
  current_millis_temp = current_millis_distance;

  if (network->firebaseReady() && (current_millis_temp - previous_millis_temp >= time_interval_temp)) {

    if (!DateTime.isTimeValid()) {
      Serial.println("Failed to get time from server, retry.");
      DateTime.begin();
    } else {
      ts = DateTime.formatUTC(DateFormatter::ISO8601);
    }

    tempsensor.wake();
    double c = (double)tempsensor.readTempC();
    Serial.print("Room temp: ");
    Serial.print(c, 4);
    Serial.println("°C\t  ");
    tempsensor.shutdown_wake(1);
    if (Serial.println(network->writeTemperatureData(c, ts))) {
      previous_millis_temp = current_millis_temp;
    }
  }


  if (network->firebaseReady() && (current_millis_distance - previous_millis_distance >= time_interval_distane)) {

    long duration, distance;

    // Trigger pulse
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    // Measure the echo time
    duration = pulseIn(ECHO_PIN, HIGH);

    // Convert the time to distance (in cm)
    distance = (duration * 0.0343) / 2;

    // Calculate the distance change since the last measurement
    long distance_change = distance - previous_distance;

    // Check if the distance change exceeds the specified interval
    if (abs(distance_change) >= distance_interval) {
      // Print the distance change to the Serial Monitor
      Serial.print("Distance Change: ");
      Serial.print(distance_change);
      Serial.println(" cm");

      // Update the previous distance and previous millis
      previous_distance = distance;
      previous_millis_distance = current_millis_distance;
    }
  }


  // Make sure to include a delay to prevent constant triggering of the interval
  delay(50);
}

void initNetwork() {
  network = new Network();
  network->initWiFi();
}

void setupDateTime() {

  DateTime.setServer("hu.pool.ntp.org");
  DateTime.setTimeZone("GMT+1");
  DateTime.begin();
  while (!DateTime.isTimeValid()) {
    DateTime.begin();
    Serial.println("Failed to get time from server.");
  }
  Serial.printf("Date Now is %s\n", DateTime.toISOString().c_str());
  Serial.printf("Timestamp is %ld\n", DateTime.now());
}