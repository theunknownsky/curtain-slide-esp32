#include <Arduino.h>
#include <WiFiManager.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define API_KEY "AIzaSyB3ekwAq5EGkH_MsXUCa1R3NxIH7KTK6zk"
#define DB_URL "https://curtainslide-test-default-rtdb.asia-southeast1.firebasedatabase.app/"

#define LED1 17
#define LED2 18
#define LED3 21

bool isWifiConnected;

FirebaseData fbdo_ledStatus, fbdo_ledBrightness, fbdo_ledColorValue, fbdo_curtainState, fbdo_curtainCloseDuration;
FirebaseAuth auth;
FirebaseConfig config;

String ctnsld_email = "test@ctnsld.co";
String ctnsld_pword = "12345678";
String ctnsld_userid = "pOMaE87MRrZD3lg2kyO306XzndR2";

String ledStatusPath = "users/pOMaE87MRrZD3lg2kyO306XzndR2/ledInfo/ledStatus";
String ledBrightnessPath = "users/pOMaE87MRrZD3lg2kyO306XzndR2/ledInfo/ledBrightness";
String ledColorValuePath = "users/pOMaE87MRrZD3lg2kyO306XzndR2/ledInfo/ledColorValue";

bool ledStatus;
int ledBrightness;
int red;
int green;
int blue;

void setup() {
  WiFiManager wm;
  // to make sure every run, the wifi is reset
  // wm.resetSettings();
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  // initializing wifi access point and connection
  isWifiConnected = wm.autoConnect("CTNSLD-69", "testPass");

  if(!isWifiConnected){
    Serial.println("Failed to connect.");
    ESP.restart();
  } else {
    Serial.println("You are now connected!");
  }
  
  config.api_key = API_KEY;
  config.database_url = DB_URL;
  auth.user.email = ctnsld_email;
  auth.user.password = ctnsld_pword;
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // stream
  if(!Firebase.RTDB.beginStream(&fbdo_ledStatus, ledStatusPath)){
    Serial.printf("Stream Error: %s", fbdo_ledStatus.errorReason().c_str());
  }
  if(!Firebase.RTDB.beginStream(&fbdo_ledBrightness, ledBrightnessPath)){
    Serial.printf("Stream Error: %s", fbdo_ledBrightness.errorReason().c_str());
  }
  if(!Firebase.RTDB.beginStream(&fbdo_ledColorValue, ledColorValuePath)){
    Serial.printf("Stream Error: %s", fbdo_ledColorValue.errorReason().c_str());
  }
}

void switchLEDStatus(bool status){
  ledStatus = status;
  digitalWrite(LED1, ledStatus);
  Serial.print("LED Status: ");
  Serial.println(ledStatus);
}

void changeLEDBrightness(int brightness){
  ledBrightness = brightness;
  Serial.print("LED Brightness: ");
  Serial.println(ledBrightness);
}

void changeLEDColor(String colorHexcode){
  long number = (long) strtol(colorHexcode.c_str(), NULL, 16);
  red = (number >> 16) & 0xFF;
  green = (number >> 8) & 0xFF;
  blue = number & 0xFF;
  Serial.println("LED Color");
  Serial.print("Red: ");
  Serial.println(red);
  Serial.print("Green: ");
  Serial.println(green);
  Serial.print("Blue: ");
  Serial.println(blue);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(Firebase.ready()){
    if(!Firebase.RTDB.readStream(&fbdo_ledStatus)){
      Serial.printf("Stream Error: %s", fbdo_ledStatus.errorReason().c_str());
    }
    if(fbdo_ledStatus.streamAvailable()){
      switchLEDStatus(fbdo_ledStatus.boolData());
    }
    if (ledStatus){
      if(!Firebase.RTDB.readStream(&fbdo_ledBrightness)){
        Serial.printf("Stream Error: %s", fbdo_ledBrightness.errorReason().c_str());
      }
      if(fbdo_ledBrightness.streamAvailable()){
        changeLEDBrightness(fbdo_ledBrightness.intData());
      }
      if(!Firebase.RTDB.readStream(&fbdo_ledColorValue)){
        Serial.printf("Stream Error: %s", fbdo_ledColorValue.errorReason().c_str());
      }
      if(fbdo_ledColorValue.streamAvailable()){
        changeLEDColor(fbdo_ledColorValue.stringData());
      }
    }
  }
}
