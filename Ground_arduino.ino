#include <SoftwareSerial.h>
#include <SPI.h>
#include <RF24.h>

// Struct must match sender (drone) exactly
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

RF24 radio(9, 10); // CE, CSN
const byte address[6] = "00001";

// If you're forwarding to ESP32 or logging, use SoftwareSerial
SoftwareSerial espSerial(2, 3); // RX, TX (to ESP32 or other)

void setup() {
  Serial.begin(9600);
  espSerial.begin(9600);

  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_LOW);
  radio.startListening();

  Serial.println("📡 Ground Station Ready. Waiting for sensor data...");
}

void loop() {
  if (radio.available()) {
    SensorData data;
    radio.read(&data, sizeof(data));

    // Print to Serial Monitor
    Serial.println("📥 Received Sensor Data:");
    Serial.print("📏 Distance: "); Serial.print(data.distance); Serial.println(" cm");
    Serial.print("🌡️ Temperature: "); Serial.print(data.temperature); Serial.println(" °C");
    Serial.print("💧 Humidity: "); Serial.print(data.humidity); Serial.println(" %");

    if (data.gps_fix) {
      Serial.println("📡 GPS Fix: ✅");
      Serial.print("📍 Latitude: "); Serial.println(data.latitude, 7);
      Serial.print("📍 Longitude: "); Serial.println(data.longitude, 7);
      Serial.print("🗻 Altitude: "); Serial.print(data.altitude); Serial.println(" m");
    } else {
      Serial.println("📡 GPS Fix: ❌ (No valid coordinates)");
    }

    if (data.heartbeat) {
      Serial.println("💓 Heartbeat: ✅ Pixhawk is alive");
    } else {
      Serial.println("💓 Heartbeat: ❌ No heartbeat");
    }

    Serial.println("-----------------------------");

    // Send to ESP32 or other system as CSV
    espSerial.print(data.distance); espSerial.print(",");
    espSerial.print(data.temperature); espSerial.print(",");
    espSerial.print(data.humidity); espSerial.print(",");
    espSerial.print(data.latitude, 7); espSerial.print(",");
    espSerial.print(data.longitude, 7); espSerial.print(",");
    espSerial.print(data.altitude); espSerial.print(",");
    espSerial.print(data.gps_fix); espSerial.print(",");
    espSerial.println(data.heartbeat); // ends with newline
  }
}