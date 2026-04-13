#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <SinricPro.h>
#include <SinricProSwitch.h>
#include <map>

// =====================================================
// USER CONFIGURATION
// =====================================================

// ---------- WiFi ----------
char ssid[] = "WIFI_NAME";
char pass[] = "WIFI_PASSWORD";

// ---------- Blynk ----------
#define BLYNK_TEMPLATE_ID   "TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "Smart Home"
#define BLYNK_AUTH_TOKEN    "BLYNK_AUTH_TOKEN"

// ---------- Sinric Pro ----------
#define APP_KEY    "SINRIC_APP_KEY"
#define APP_SECRET "SINRIC_APP_SECRET"

// Device IDs from Sinric Pro
#define DEVICE_ID_RED     "DEVICE_ID_RED"
#define DEVICE_ID_BLUE    "DEVICE_ID_BLUE"
#define DEVICE_ID_YELLOW  "DEVICE_ID_YELLOW"
#define DEVICE_ID_FAN     "DEVICE_ID_FAN"

// =====================================================
// PIN DEFINITIONS (ESP8266 NodeMCU)
// =====================================================

// Relays (active LOW)
#define RELAY_RED_PIN     D1   // GPIO5
#define RELAY_BLUE_PIN    D2   // GPIO4
#define RELAY_YELLOW_PIN  D5   // GPIO14

// Fan / Motor output
#define FAN_PWM_PIN       D6   // GPIO12
#define FAN_ENABLE_PIN    D8   // GPIO15

// Manual switches (to GND)
#define SW_RED_PIN        D3   // GPIO0
#define SW_BLUE_PIN       D4   // GPIO2
#define SW_YELLOW_PIN     D0   // GPIO16

// IR receiver
#define IR_PIN            D7   // GPIO13

// DHT11
#define DHT_PIN           3    // RX pin (GPIO3)
#define DHT_TYPE          DHT11

// =====================================================
// SETTINGS
// =====================================================
#define RELAY_ON          LOW
#define RELAY_OFF         HIGH

#define SWITCH_PRESSED    LOW
#define DEBOUNCE_MS       250

#define FAN_MIN_PWM       0
#define FAN_MAX_PWM       255

BlynkTimer timer;
DHT dht(DHT_PIN, DHT_TYPE);
IRrecv irrecv(IR_PIN);
decode_results results;

// =====================================================
// STATE VARIABLES
// =====================================================
bool redState = false;
bool blueState = false;
bool yellowState = false;
bool fanState = false;

int fanPWM = 0;

float temperatureC = 0.0;
float humidityRH = 0.0;

unsigned long lastRedSwitchTime = 0;
unsigned long lastBlueSwitchTime = 0;
unsigned long lastYellowSwitchTime = 0;

bool lastRedSwitchReading = HIGH;
bool lastBlueSwitchReading = HIGH;
bool lastYellowSwitchReading = HIGH;

// =====================================================
// IR REMOTE CODES
// Change these if your remote uses different values
// =====================================================
const uint32_t IR_RED_ON       = 0x1801;
const uint32_t IR_RED_OFF      = 0x1001;
const uint32_t IR_BLUE_ON      = 0x1802;
const uint32_t IR_BLUE_OFF     = 0x1002;
const uint32_t IR_YELLOW_ON    = 0x1803;
const uint32_t IR_YELLOW_OFF   = 0x1003;
const uint32_t IR_FAN_ON       = 0x1804;
const uint32_t IR_FAN_OFF      = 0x1004;

// =====================================================
// HELPER FUNCTIONS
// =====================================================
void setRelay(uint8_t pin, bool state) {
  digitalWrite(pin, state ? RELAY_ON : RELAY_OFF);
}

void applyOutputs() {
  setRelay(RELAY_RED_PIN, redState);
  setRelay(RELAY_BLUE_PIN, blueState);
  setRelay(RELAY_YELLOW_PIN, yellowState);

  if (fanState) {
    digitalWrite(FAN_ENABLE_PIN, HIGH);
    analogWrite(FAN_PWM_PIN, fanPWM);
  } else {
    digitalWrite(FAN_ENABLE_PIN, LOW);
    analogWrite(FAN_PWM_PIN, 0);
  }
}

void syncBlynkWidgets() {
  Blynk.virtualWrite(V0, redState);
  Blynk.virtualWrite(V1, blueState);
  Blynk.virtualWrite(V2, yellowState);
  Blynk.virtualWrite(V4, fanState);
  Blynk.virtualWrite(V3, temperatureC);
  Blynk.virtualWrite(V5, humidityRH);
}

void updateTemperatureHumidity() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (!isnan(h)) humidityRH = h;
  if (!isnan(t)) temperatureC = t;

  // Same idea as your original code: PWM depends on temperature
  fanPWM = constrain((int)(temperatureC * 5.1), FAN_MIN_PWM, FAN_MAX_PWM);

  if (fanState) {
    analogWrite(FAN_PWM_PIN, fanPWM);
  }

  Blynk.virtualWrite(V3, temperatureC);
  Blynk.virtualWrite(V5, humidityRH);
}

void toggleRed() {
  redState = !redState;
  applyOutputs();
  syncBlynkWidgets();
}

void toggleBlue() {
  blueState = !blueState;
  applyOutputs();
  syncBlynkWidgets();
}

void toggleYellow() {
  yellowState = !yellowState;
  applyOutputs();
  syncBlynkWidgets();
}

void setFanState(bool state) {
  fanState = state;
  applyOutputs();
  syncBlynkWidgets();
}

// =====================================================
// SINRIC PRO
// =====================================================
bool onPowerState(const String &deviceId, bool &state) {
  if (deviceId == DEVICE_ID_RED) {
    redState = state;
  } else if (deviceId == DEVICE_ID_BLUE) {
    blueState = state;
  } else if (deviceId == DEVICE_ID_YELLOW) {
    yellowState = state;
  } else if (deviceId == DEVICE_ID_FAN) {
    fanState = state;
  } else {
    return false;
  }

  applyOutputs();
  syncBlynkWidgets();
  return true;
}

void setupSinricPro() {
  SinricProSwitch &redSwitch = SinricPro[DEVICE_ID_RED];
  SinricProSwitch &blueSwitch = SinricPro[DEVICE_ID_BLUE];
  SinricProSwitch &yellowSwitch = SinricPro[DEVICE_ID_YELLOW];
  SinricProSwitch &fanSwitch = SinricPro[DEVICE_ID_FAN];

  redSwitch.onPowerState(onPowerState);
  blueSwitch.onPowerState(onPowerState);
  yellowSwitch.onPowerState(onPowerState);
  fanSwitch.onPowerState(onPowerState);

  SinricPro.begin(APP_KEY, APP_SECRET);
  SinricPro.restoreDeviceStates(true);
}

// =====================================================
// BLYNK HANDLERS
// =====================================================
BLYNK_WRITE(V0) {
  redState = param.asInt();
  applyOutputs();
}

BLYNK_WRITE(V1) {
  blueState = param.asInt();
  applyOutputs();
}

BLYNK_WRITE(V2) {
  yellowState = param.asInt();
  applyOutputs();
}

BLYNK_WRITE(V4) {
  fanState = param.asInt();
  applyOutputs();
}

// =====================================================
// MANUAL SWITCH HANDLING
// =====================================================
void checkManualSwitch(
  uint8_t pin,
  bool &lastReading,
  unsigned long &lastTime,
  void (*toggleFunction)()
) {
  bool reading = digitalRead(pin);

  if (reading != lastReading) {
    lastTime = millis();
    lastReading = reading;
  }

  if ((millis() - lastTime) > DEBOUNCE_MS) {
    static std::map<uint8_t, bool> handled;
    if (reading == SWITCH_PRESSED && !handled[pin]) {
      toggleFunction();
      handled[pin] = true;
    } else if (reading != SWITCH_PRESSED) {
      handled[pin] = false;
    }
  }
}

void handleManualSwitches() {
  checkManualSwitch(SW_RED_PIN, lastRedSwitchReading, lastRedSwitchTime, toggleRed);
  checkManualSwitch(SW_BLUE_PIN, lastBlueSwitchReading, lastBlueSwitchTime, toggleBlue);
  checkManualSwitch(SW_YELLOW_PIN, lastYellowSwitchReading, lastYellowSwitchTime, toggleYellow);
}

// =====================================================
// IR HANDLING
// =====================================================
void handleIRRemote() {
  if (!irrecv.decode(&results)) return;

  uint32_t code = results.value;

  if (code == IR_RED_ON) redState = true;
  else if (code == IR_RED_OFF) redState = false;
  else if (code == IR_BLUE_ON) blueState = true;
  else if (code == IR_BLUE_OFF) blueState = false;
  else if (code == IR_YELLOW_ON) yellowState = true;
  else if (code == IR_YELLOW_OFF) yellowState = false;
  else if (code == IR_FAN_ON) fanState = true;
  else if (code == IR_FAN_OFF) fanState = false;

  applyOutputs();
  syncBlynkWidgets();

  irrecv.resume();
}

// =====================================================
// WIFI + SETUP
// =====================================================
void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
  }
}

void setupPins() {
  pinMode(RELAY_RED_PIN, OUTPUT);
  pinMode(RELAY_BLUE_PIN, OUTPUT);
  pinMode(RELAY_YELLOW_PIN, OUTPUT);

  pinMode(FAN_PWM_PIN, OUTPUT);
  pinMode(FAN_ENABLE_PIN, OUTPUT);

  pinMode(SW_RED_PIN, INPUT_PULLUP);
  pinMode(SW_BLUE_PIN, INPUT_PULLUP);
  pinMode(SW_YELLOW_PIN, INPUT_PULLUP);

  // Default outputs OFF
  digitalWrite(RELAY_RED_PIN, RELAY_OFF);
  digitalWrite(RELAY_BLUE_PIN, RELAY_OFF);
  digitalWrite(RELAY_YELLOW_PIN, RELAY_OFF);

  digitalWrite(FAN_ENABLE_PIN, LOW);
  analogWrite(FAN_PWM_PIN, 0);
}

void setup() {
  setupPins();

  dht.begin();
  irrecv.enableIRIn();

  connectWiFi();
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  setupSinricPro();

  timer.setInterval(2000L, updateTemperatureHumidity);
  timer.setInterval(1000L, syncBlynkWidgets);

  applyOutputs();
  syncBlynkWidgets();
}

// =====================================================
// MAIN LOOP
// =====================================================
void loop() {
  Blynk.run();
  timer.run();
  SinricPro.handle();

  handleManualSwitches();
  handleIRRemote();
}
