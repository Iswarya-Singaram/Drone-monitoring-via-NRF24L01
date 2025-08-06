#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>

// SoftwareSerial pins (D6 = RX, D7 = TX on NodeMCU)
#define RX 12
#define TX 13
SoftwareSerial arduinoSerial(RX, TX);

float distance, temperature, humidity;
float latitude, longitude, altitude;

void setup() {
  Serial.begin(115200);           // Debug monitor
  arduinoSerial.begin(9600);      // Communication from Arduino

  Serial.println("📡 Waiting for sensor + GPS data from GCS Arduino...");
}

void loop() {
  if (arduinoSerial.available()) {
    String line = arduinoSerial.readStringUntil('\n');
    line.trim(); // Remove any trailing newline or spaces

    Serial.print("🔄 Raw received: ");
    Serial.println(line);

    // Expecting: distance,temp,humidity,lat,lon,alt
    int indices[6];
    int idx = -1;
    for (int i = 0; i < 6; i++) {
      idx = line.indexOf(',', idx + 1);
      indices[i] = idx;
    }

    if (indices[4] > 0) {
      // Parse values
      distance   = line.substring(0, indices[0]).toFloat();
      temperature= line.substring(indices[0] + 1, indices[1]).toFloat();
      humidity   = line.substring(indices[1] + 1, indices[2]).toFloat();
      latitude   = line.substring(indices[2] + 1, indices[3]).toFloat();
      longitude  = line.substring(indices[3] + 1, indices[4]).toFloat();
      altitude   = line.substring(indices[4] + 1).toFloat();

      // Print parsed values
      Serial.println("✅ Parsed data:");
      Serial.print("Distance: "); Serial.print(distance); Serial.println(" cm");
      Serial.print("Temperature: "); Serial.print(temperature); Serial.println(" °C");
      Serial.print("Humidity: "); Serial.print(humidity); Serial.println(" %");
      Serial.print("Latitude: "); Serial.println(latitude, 7);
      Serial.print("Longitude: "); Serial.println(longitude, 7);
      Serial.print("Altitude: "); Serial.print(altitude); Serial.println(" m");
      Serial.println("---------------------------------------------------");
    } else {
      Serial.println("⚠️ Parsing failed. Ensure CSV format is correct.");
    }
  }
}