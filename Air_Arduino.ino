#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <DHT.h>
#include <SoftwareSerial.h>
#include "common/mavlink.h"  // Make sure this points to your MAVLink path

// Pin Definitions
#define DHTPIN 5
#define DHTTYPE DHT11
#define TRIG_PIN 7
#define ECHO_PIN 6

// DHT & Distance
DHT dht(DHTPIN, DHTTYPE);

// NRF24L01 setup
RF24 radio(9, 10); // CE, CSN
const byte address[6] = "00001";

// MAVLink
SoftwareSerial mavSerial(2, 3);  // RX = 2 (Pixhawk TX), TX = 3 (unused)

// Sensor + GPS Data Struct
struct SensorData {
  float distance;
  float temperature;
  float humidity;
  float latitude;
  float longitude;
  float altitude;
  bool gps_fix;
  bool heartbeat;
};

SensorData data;

void setup() {
  Serial.begin(9600);

  // DHT & Distance
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  dht.begin();

  // NRF24
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_LOW);
  radio.stopListening();

  // MAVLink
  mavSerial.begin(57600);
  Serial.println("📡 Listening for MAVLink messages from Pixhawk...");
}

float readDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH, 30000); // Timeout after 30ms
  return duration > 0 ? duration * 0.034 / 2 : -1.0;
}

void loop() {
  // Update local sensor data
  data.distance = readDistance();
  data.temperature = dht.readTemperature();
  data.humidity = dht.readHumidity();

  // Default values
  data.latitude = 0;
  data.longitude = 0;
  data.altitude = 0;
  data.gps_fix = false;
  data.heartbeat = false;

  // Parse MAVLink messages (non-blocking)
  mavlink_message_t msg;
  mavlink_status_t status;

  unsigned long startTime = millis();
  while (millis() - startTime < 200) { // Read MAVLink for 200ms
    if (mavSerial.available()) {
      uint8_t c = mavSerial.read();
      if (mavlink_parse_char(MAVLINK_COMM_0, c, &msg, &status)) {
        Serial.print("📦 Received MAVLink msgid: ");
        Serial.println(msg.msgid);

        if (msg.msgid == MAVLINK_MSG_ID_HEARTBEAT) {
          Serial.println("✅ Heartbeat received from Pixhawk");
          data.heartbeat = true;
        }

        if (msg.msgid == MAVLINK_MSG_ID_GLOBAL_POSITION_INT) {
          mavlink_global_position_int_t pos;
          mavlink_msg_global_position_int_decode(&msg, &pos);

          data.latitude = pos.lat / 1e7;
          data.longitude = pos.lon / 1e7;
          data.altitude = pos.alt / 1000.0;
          data.gps_fix = true;

          Serial.println("📡 GLOBAL_POSITION_INT Received:");
          Serial.print("Latitude: "); Serial.println(data.latitude, 7);
          Serial.print("Longitude: "); Serial.println(data.longitude, 7);
          Serial.print("Altitude: "); Serial.print(data.altitude); Serial.println(" m");
        }
      }
    }
  }

  // Send data over RF24
  radio.write(&data, sizeof(data));

  // Print locally for debug
  Serial.print("📤 Sent -> D: "); Serial.print(data.distance);
  Serial.print(" cm | T: "); Serial.print(data.temperature);
  Serial.print(" °C | H: "); Serial.print(data.humidity);
  Serial.print(" % | Fix: "); Serial.print(data.gps_fix);
  Serial.print(" | Lat: "); Serial.print(data.latitude, 7);
  Serial.print(" | Lon: "); Serial.print(data.longitude, 7);
  Serial.print(" | Alt: "); Serial.print(data.altitude);
  Serial.print(" | HB: "); Serial.println(data.heartbeat);

  delay(1000);
}