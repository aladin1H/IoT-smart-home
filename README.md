**IoT Smart Home System**

This project implements a smart home automation system using ESP8266 NodeMCU. It allows control of home appliances using multiple methods including Blynk mobile app, Alexa (via SinricPro), IR remote, and physical switches. The system also monitors temperature and humidity and automatically adjusts fan speed.

**Features**
Control 3 appliances (relays) and 1 fan/motor
Multiple control methods:
Blynk mobile app
Alexa voice control
IR remote
Manual switches
Real-time temperature and humidity monitoring (DHT11)
Automatic fan speed control based on temperature
Real-time synchronization across all control systems

**System Overview**

User Input (Blynk / Alexa / IR / Switches) → ESP8266 → Relays & Motor → Appliances

**Hardware Requirements**
ESP8266 NodeMCU
3-channel Relay Module
DC Motor / Fan with driver
DHT11 Sensor
IR Receiver
3 Push Buttons (manual switches)
Power Supply
Jumper wires

**Pin Configuration**
Outputs
Red Relay → D1 (GPIO5)
Blue Relay → D2 (GPIO4)
Yellow Relay → D5 (GPIO14)
Fan PWM → D6 (GPIO12)
Fan Enable → D8 (GPIO15)
Inputs
Red Switch → D3 (GPIO0)
Blue Switch → D4 (GPIO2)
Yellow Switch → D0 (GPIO16)
IR Receiver → D7 (GPIO13)
DHT11 Data → RX (GPIO3)

**Working Principle**

Relay Control
Each relay can be controlled through app, voice, IR remote, or physical switch. All control methods remain synchronized.

Fan Control
Fan can be turned ON/OFF manually and its speed is automatically adjusted based on temperature:

PWM = Temperature × 5.1

Sensor Monitoring
DHT11 continuously measures temperature and humidity and sends data to Blynk.

IR Control
IR remote sends predefined codes to toggle devices.

Manual Switches
Uses INPUT_PULLUP configuration:

Pressed = LOW
Released = HIGH

**Blynk Setup**
V0 → Red Relay
V1 → Blue Relay
V2 → Yellow Relay
V3 → Temperature
V4 → Fan Control
V5 → Humidity

**Setup Instructions**
Install required libraries:
Blynk
SinricPro
IRremoteESP8266
DHT sensor library
Update credentials in code:
WiFi name and password
Blynk auth token
SinricPro keys
Upload code to ESP8266
Configure Blynk app widgets
Setup Alexa using SinricPro

**Important Notes**
Relays are active LOW
Use common ground for all components
Do not upload API keys or WiFi credentials to GitHub
Ensure stable power supply

**Future Improvements**
Add scheduling system
Add energy monitoring
Add mobile notifications
Integrate Home Assistant
Improve UI dashboard
