//#include "DHT.h"        // including the library of DHT11 temperature and humidity sensors 
#define DHTTYPE DHT11   // DHT 11 
#include <IRremote.h> 
int RECV_PIN = 2; 
IRrecv irrecv(RECV_PIN); 
decode_results  results; 
#define BLYNK_PRINT Serial 
#define PIN_LED_RED 16 
#define PIN_LED_BLUE 5 
#define PIN_LED_YELLOW 4 
//#define MOTOR 0 
#define PIN_RED_SWITCH 14 
#define PIN_BLUE_SWITCH 1 
#define PIN_YELLOW_SWITCH 13 
/* Fill in information from Blynk Device Info here */ 
//#define BLYNK_TEMPLATE_ID           "TMPxxxxxx" 
//#define BLYNK_TEMPLATE_NAME         "Device" 
//#define BLYNK_AUTH_TOKEN
            "YourAuthToken" 
#define BLYNK_TEMPLATE_ID "TMPL6Zm1F-ATp" 
#define BLYNK_TEMPLATE_NAME "Smart Home " 
45 
#define BLYNK_AUTH_TOKEN "_CKrsn86A6FDfW1GucxVpHpoJlsGu9tJ" 
#include <ESP8266WiFi.h> 
#include <BlynkSimpleEsp8266.h> 
#define dht_dpin 15 
#ifdef ENABLE_DEBUG 
#define DEBUG_ESP_PORT Serial 
#define NODEBUG_WEBSOCKETS 
#define NDEBUG 
#endif  
#include <Arduino.h> 
#include <ESP8266WiFi.h> 
#include "SinricPro.h" 
#include "SinricProSwitch.h" 
#include <map> 
char ssid[] = "A50"; 
char pass[] = "22222222"; 
BlynkTimer timer; 
#define WIFI_SSID
 #define WIFI_PASS        
#define APP_KEY
         "A50"     
 "22222222" 
           "1a001925-d48b-44b6-9382-fbfc2fd98f60"      // Should look like 
"de0bxxxx-1x3x-4x3x-ax2x-5dabxxxxxxxx" 
46 
#define APP_SECRET    
    "239aeaae-2bcc-4f1b-8f5e-aab1d407de47-b8471fd1-c588-4eb1
9b18-dc16ceaac084"   // Should look like "5f36xxxx-x3x7-4x3x-xexe-e86724a9xxxx
4c4axxxx-3x3x-x5xe-x9x3-333d65xxxxxx" 
//Enter the device IDs here 
#define device_ID_1   "xxxxxxxxxxxxxxxxxxxxxxxx" 
#define device_ID_2   "65d39ad0ccc93539a13da802" 
#define device_ID_3   "65d3a164b0a460b25b472231" 
#define device_ID_4   "65d3a2adccc93539a13dad05" 
// define the GPIO connected with Relays and switches 
#define RelayPin1 10  //D1 
#define RelayPin2 16  //D2 
#define RelayPin3 5 //D5 
#define RelayPin4 4 //D6 
#define SwitchPin1 0  //SD3 
#define SwitchPin2 14   //D3  
#define SwitchPin3 1  //D7 
#define SwitchPin4 13   //RX 
#define wifiLed   16   //D0 
// comment the following line if you use a toggle switches instead of tactile buttons 
//#define TACTILE_BUTTON 1 
#define BAUD_RATE   9600 
#define DEBOUNCE_TIME 250 
DHT dht(dht_dpin, DHTTYPE);  
47 
typedef struct {      // struct for the std::map below 
int relayPIN; int flipSwitchPIN;} deviceConfig_t; 
// this is the main configuration 
// please put in your deviceId, the PIN for Relay and PIN for flipSwitch 
// this can be up to N devices...depending on how much pin's available on your device ;) 
// right now we have 4 devicesIds going to 4 relays and 4 flip switches to switch the relay 
manually 
std::map<String, deviceConfig_t> devices = { 
//{deviceId, {relayPIN,  flipSwitchPIN}} 
//{device_ID_1, {  RelayPin1, SwitchPin1 }}, 
{device_ID_2, {  RelayPin2, SwitchPin2 }}, 
{device_ID_3, {  RelayPin3, SwitchPin3 }}, 
{device_ID_4, {  RelayPin4, SwitchPin4 }}      
}; 
typedef struct {      // struct for the std::map below 
String deviceId;bool lastFlipSwitchState; 
unsigned long lastFlipSwitchChange;} flipSwitchConfig_t; 
std::map<int, flipSwitchConfig_t> flipSwitches;    // this map is used to map flipSwitch PINs 
to deviceId and handling debounce and last flipSwitch state checks 
// it will be setup in "setupFlipSwitches" function, using informations from devices map 
void setupRelays() {  
for (auto &device : devices) {           // for each device (relay, flipSwitch combination) 
int relayPIN = device.second.relayPIN; // get the relay pin 
48 
pinMode(relayPIN, OUTPUT);             // set relay pin to OUTPUT 
digitalWrite(relayPIN, LOW);  }} 
void setupFlipSwitches() { 
for (auto &device : devices)  {        // for each device (relay / flipSwitch combination) 
flipSwitchConfig_t flipSwitchConfig;            // create a new flipSwitch configuration 
flipSwitchConfig.deviceId = device.first;
         //
 set the deviceId 
flipSwitchConfig.lastFlipSwitchChange = 0;        // set debounce time 
flipSwitchConfig.lastFlipSwitchState = true;     // set lastFlipSwitchState to false (LOW)-- 
int flipSwitchPIN = device.second.flipSwitchPIN;  // get the flipSwitchPIN 
flipSwitches[flipSwitchPIN] = flipSwitchConfig;   // save the flipSwitch config to 
flipSwitches map 
pinMode(flipSwitchPIN, INPUT_PULLUP);    
INPUT}} 
int ult_red,ult_blue,ult_yellow,ult_motor; 
               //
 set the flipSwitch pin to 
int sin_blue,sin_red,sin_yellow,sin_red_old,sin_blue_old,sin_yellow_old 
bool onPowerState(String deviceId, bool &state) 
{ 
Serial.printf("%s: %s\r\n", deviceId.c_str(), state ? "on" : "off"); 
int relayPIN = devices[deviceId].relayPIN; // get the relay pin for corresponding device 
//digitalWrite(relayPIN, !state);    
sin_red_old=sin_red;sin_blue_old=sin_blue; 
sin_yellow_old=sin_yellow;          
// set the new relay state 
if(relayPIN==PIN_LED_RED) sin_red=state; 
49 
if(relayPIN==PIN_LED_BLUE) sin_blue=state; 
if(relayPIN==PIN_LED_YELLOW) sin_yellow=state; 
if(sin_blue!=sin_blue_old) ult_blue=sin_blue; 
if(sin_red!=sin_red_old) ult_red=sin_red; 
if(sin_yellow!=sin_yellow_old) ult_yellow=sin_yellow; return true;} 
void handleFlipSwitches() {unsigned long actualMillis = millis();                         // get actual 
millis for (auto &flipSwitch : flipSwitches) {                    // for each flipSwitch in flipSwitches 
map unsigned long lastFlipSwitchChange = flipSwitch.second.lastFlipSwitchChange;  // get 
the timestamp when flipSwitch was pressed last time (used to debounce / limit events) 
if (actualMillis - lastFlipSwitchChange > DEBOUNCE_TIME) {     // if time is > debounce 
time...int flipSwitchPIN = flipSwitch.first;
        //
 get the flipSwitch pin from configuration 
bool lastFlipSwitchState = flipSwitch.second.lastFlipSwitchState;     // get the 
lastFlipSwitchState 
bool flipSwitchState = digitalRead(flipSwitchPIN);   // read the current flipSwitch state 
if (flipSwitchState != lastFlipSwitchState) {     // if the flipSwitchState has changed... 
#ifdef TACTILE_BUTTON 
if (flipSwitchState) {       // if the tactile button is pressed  
#endif      flipSwitch.second.lastFlipSwitchChange = actualMillis;              
lastFlipSwitchChange time 
String deviceId = flipSwitch.second.deviceId;    
                       //
    // update 
 get the deviceId from config 
int relayPIN = devices[deviceId].relayPIN;                              // get the relayPIN from config   
bool newRelayState = !digitalRead(relayPIN);       
                     //
 digitalWrite(relayPIN, newRelayState);                  
                //
 set the new relay State 
 set the trelay to the new state 
SinricProSwitch &mySwitch = SinricPro[deviceId];         // get Switch device from SinricPro 
mySwitch.sendPowerStateEvent(!newRelayState);                      // send the event 
50 
#ifdef TACTILE_BUTTON } 
#endif       flipSwitch.second.lastFlipSwitchState = flipSwitchState;    // update 
lastFlipSwitchState} } }} 
void setupWiFi() 
{ Serial.printf("\r\n[Wifi]: Connecting"); 
WiFi.begin(WIFI_SSID, WIFI_PASS); 
while (WiFi.status() != WL_CONNECTED)  { 
Serial.printf("."); delay(250);  }digitalWrite(wifiLed, LOW); 
Serial.printf("connected!\r\n[WiFi]: IP-Address is %s\r\n", WiFi.localIP().toString().c_str());} 
void setupSinricPro(){  for (auto &device : devices) {const char *deviceId = 
device.first.c_str();SinricProSwitch &mySwitch = SinricPro[deviceId]; 
mySwitch.onPowerState(onPowerState); } 
SinricPro.begin(APP_KEY, APP_SECRET); SinricPro.restoreDeviceStates(true);} 
void setup(){dht.begin();Serial.begin(BAUD_RATE);  //irrecv.enableIRIn(); 
pinMode(wifiLed, OUTPUT);digitalWrite(wifiLed, HIGH); setupRelays(); 
setupFlipSwitches(); setupWiFi(); setupSinricPro();  // Debug console Serial.begin(9600); 
pinMode(PIN_LED_RED, OUTPUT); 
pinMode(PIN_LED_BLUE, OUTPUT); 
pinMode(PIN_LED_YELLOW, OUTPUT); 
pinMode(12,OUTPUT); pinMode(0,OUTPUT);digitalWrite(12,LOW);digitalWrite(0,LOW); 
//pinMode(PIN_LED_3, OUTPUT); 
Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass); 
timer.setInterval(500, updateBlynk);//delay(5000); 
51 
digitalWrite(PIN_LED_RED,LOW); digitalWrite(PIN_LED_BLUE,LOW); 
digitalWrite(PIN_LED_YELLOW,LOW); //digitalWrite(PIN_LED_3,HIGH); 
Blynk.virtualWrite(V1, 0);Blynk.virtualWrite(V2, 0);  Blynk.virtualWrite(V3, 0); 
irrecv.enableIRIn();}float h,t;//int v0,v1,v2;int 
v0=0,v1=0,v2=0,v3=0,v0_old,v1_old,v2_old;int 
s_red,s_blue,s_yellow,s_red_old,s_blue_old,s_yellow_old,r_red,r_red_old,r_blue,r_blue_old,r
 _yellow,r_yellow_old,r_mot,r_mot_old,ult_pwm=0; 
void loop() 
{h = dht.readHumidity(); t = dht.readTemperature();  ult_pwm=t*5.1; 
s_red_old=s_red;  s_blue_old=s_blue; s_yellow_old=s_yellow;  
s_red=digitalInput(PIN_RED_SWITCH);  s_blue=digitalInput(PIN_BLUE_SWITCH); 
s_yellow=digitalInput(PIN_YELLOW_SWITCH);  if(s_red_old!=s_red) ult_red=s_red; 
if(s_blue_old!=s_blue) ult_blue= s_blue; if(s_yellow_old!=s_yellow) ult_yellow=s_yellow; 
if (irrecv.decode(&results))  {  Serial.println(results.value, HEX); irrecv.resume(); // Receive  
the next value  r_red_old=r_red; r_blue_old=r_blue;  r_yellow_old=r_yellow;  
r_mot_old=r_mot; 
if(results.value==0x1801){  r_red=1; }else if(results.value==0x1001){  r_red=0;  } 
if(results.value==0x1802){ r_blue=1;    }else if(results.value==0x1002){   r_blue=0;} 
if(results.value==0x1803){  r_yellow=1;   }else if(results.value==0x1003){   r_yellow=0;  } 
if(results.value==0x1804){ r_mot=1;  }else if(results.value==0x1004){ r_mot=0;   } 
if(r_mot_old!=r_mot) ult_motor=r_mot; if(r_red_old!=r_red) ult_red=r_red; 
if(r_blue_old!=r_blue) ult_blue=r_blue; if(r_yellow_old!=r_yellow) ult_yellow=r_yellow;} 
SinricPro.handle();handleFlipSwitches();  //Serial.println("/////"); 
//Serial.println(ult_red);//Serial.println(ult_blue); 
//Serial.println(ult_yellow);Serial.println(t); Blynk.run();timer.run();} 
52 
BLYNK_WRITE(V0){v0_old=v0; v0=param.asInt();  if(v0_old!=v0) ult_red=v0;  } 
BLYNK_WRITE(V1){ v1_old=v1;v1=param.asInt();if(v1_old!=v1) ult_blue=v1;} 
BLYNK_WRITE(V2) 
{v2_old=v2;v2=param.asInt();   if(v2_old!=v2) {ult_yellow=v2;//ult_motor=v2;}} 
int v5,v5_old; 
BLYNK_WRITE(V4){v5_old=v5; v5=param.asInt(); if(v5_old!=v5) ult_motor=v5;} 
void turnOnLED(int pin) { 
digitalWrite(pin, HIGH); 
//Blynk.virtualWrite(pin, 1);}void turnOffLED(int pin) { digitalWrite(pin, LOW); 
//Blynk.virtualWrite(pin, 1);} 
void updateBlynk() {  //Serial.println(bl1);if (ult_red == 1) {turnOnLED(PIN_LED_RED);} 
else {turnOffLED(PIN_LED_RED);} 
if (ult_blue == 1) { turnOnLED(PIN_LED_BLUE);//Serial.println("Led_1_ON..."); } else { 
turnOffLED(PIN_LED_BLUE);//Serial.println("LED_1_OFF....");  }if (ult_yellow == 1) { 
turnOnLED(PIN_LED_YELLOW); } else {  turnOffLED(PIN_LED_YELLOW);} 
if (ult_motor == 1) {digitalWrite(12,HIGH);analogWrite(0,ult_pwm); } else 
{digitalWrite(12,LOW);analogWrite(0,0);  }Blynk.virtualWrite(V3,t);  
//Blynk.virtualWrite(V4,h);} 
int digitalInput(int pin){  
pinMode(pin,OUTPUT);digitalWrite(pin,HIGH);pinMode(pin,INPUT); 
return digitalRead(pin); 
} 
