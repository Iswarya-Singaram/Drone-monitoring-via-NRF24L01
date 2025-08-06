# 🛰️ Drone Telemetry System with MAVLink, RF24, and ESP Web Dashboard

This project is a **drone telemetry pipeline** that collects **sensor and GPS data** from a Pixhawk-equipped drone and transmits it wirelessly to a **Ground Control Station (GCS)** and ultimately to a **web dashboard**.

---

### 🔧 Project Overview

- The **Drone Arduino** reads:
  - MAVLink data (GPS & heartbeat) from Pixhawk
  - Temperature & humidity via DHT11
  - Distance via ultrasonic sensor
  - Sends this over **NRF24L01+** to the Ground Arduino

- The **Ground Arduino** receives this and **either**:
  - Forwards it via UART to an **ESP8266 (NodeMCU)** *(optional)*
  - **Or** connects **directly via USB** to a Flask web server

- The **ESP8266 or direct USB** sends telemetry to a **Flask web server**, which:
  - Parses and displays live telemetry via **WebSockets**
  - Tracks **GPS path** (with fix only)
  - Provides a REST endpoint `/data` for integration

---

## ⚙️ Optional ESP8266

> You **can skip** the ESP8266 if you connect the **Ground Arduino directly to your PC** (running the Flask server) via USB.

### ✅ Two Setup Options:

| Setup Option | Description |
|--------------|-------------|
| **With ESP8266** | Arduino → ESP8266 → Flask Server via UART |
| **Without ESP8266** | Arduino → PC USB → Flask Server (direct serial read) |

This gives you flexibility based on whether you want wireless transmission to the server or a USB tethered setup.

---

## 📦 Hardware Connections

### 🚁 Drone Side (Transmitting Arduino)

| Component        | Arduino Pin | Notes                            |
|------------------|-------------|----------------------------------|
| DHT11            | D5          | Data pin                         |
| Ultrasonic TRIG  | D7          | Trigger pin                      |
| Ultrasonic ECHO  | D6          | Echo pin                         |
| NRF24L01 CE      | D9          |                                  |
| NRF24L01 CSN     | D10         |                                  |
| NRF24L01 MOSI    | D11         | Hardware SPI                     |
| NRF24L01 MISO    | D12         | Hardware SPI                     |
| NRF24L01 SCK     | D13         | Hardware SPI                     |
| Pixhawk TX       | D2 (RX)     | Connect Pixhawk TELEM TX         |

> **Note:** Use `SoftwareSerial` for Pixhawk MAVLink communication. Baud: 57600.

---

### 🖥️ Ground Side (Receiving Arduino)

| Component        | Arduino Pin | Notes                            |
|------------------|-------------|----------------------------------|
| NRF24L01 CE      | D9          | Same as above                    |
| NRF24L01 CSN     | D10         | Same as above                    |
| NRF24L01 MOSI    | D11         | Hardware SPI                     |
| NRF24L01 MISO    | D12         | Hardware SPI                     |
| NRF24L01 SCK     | D13         | Hardware SPI                     |
| To ESP8266 TX    | D2 (RX)     | `SoftwareSerial` to ESP RX       |
| To ESP8266 RX    | D3 (TX)     | `SoftwareSerial` to ESP TX       |
| **OR USB to PC** | USB Port    | Use for direct Flask connection  |

---

### 🌐 ESP8266 (NodeMCU) *(Optional)*

| NodeMCU Pin | Connected To (GCS Arduino) | Notes               |
|-------------|----------------------------|---------------------|
| D6 (GPIO12) | Arduino D3 (TX)            | NodeMCU RX (Soft)   |
| D7 (GPIO13) | Arduino D2 (RX)            | NodeMCU TX (Soft)   |

> **Note:** ESP8266 reads serial data and forwards to Flask server.

---

## 🖥️ Web Server

- **Language:** Python 3
- **Framework:** Flask + Flask-SocketIO
- **Serial Interface:** Auto-detects CH340/Arduino or ESP UART
- **Displays:**
  - Temperature, Humidity, Distance
  - GPS Coordinates (if GPS fix)
  - Heartbeat status
  - History of GPS path
- **Endpoints:**
  - `/`: Real-time dashboard (via `index.html`)
  - `/data`: JSON of current and history

---

## ✅ To-Do / Improvements

- Add GPS map plotting in HTML (Leaflet.js or Google Maps)
- Add SD card logging on GCS
- Expand to include more MAVLink types (Battery, Attitude)
- Optional: Add camera streaming module

---

## 📷 Preview

Coming soon...

---

## 📜 License

MIT License. Free to use and modify with credit.

---

Made with ❤️ for drones, sensors, and code.
