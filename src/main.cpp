#include <Arduino.h>
#include <WiFiManager.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include <credentials.h>


#define LED1 17

#define SWITCH_CLOSE 18
#define SWITCH_OPEN 19

bool isWifiConnected;

FirebaseData fbdo_ledStatus, fbdo_ledBrightness, fbdo_ledColorValue, fbdo_curtainState, fbdo_curtainCloseDuration;
FirebaseAuth auth;
FirebaseConfig config;

bool ledStatus;
int ledBrightness;
int red;
int green;
int blue;
int curtainState;
int curtainCloseDuration;
int switchOpen;
int switchClose;


void setup() {
  WiFiManager wm;
  // to make sure every run, the wifi is reset
  // wm.resetSettings();
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(LED1, OUTPUT);
  pinMode(SWITCH_CLOSE, INPUT);
  pinMode(SWITCH_OPEN, INPUT);
  // initializing wifi access point and connection
  isWifiConnected = wm.autoConnect(AP_SSID, AP_PASSWORD);

  if(!isWifiConnected){
    Serial.println("Failed to connect.");
    ESP.restart();
  } else {
    Serial.println("You are now connected!");
  }
  
  config.api_key = API_KEY;
  config.database_url = DB_URL;
  auth.user.email = CTNSLD_EMAIL;
  auth.user.password = CTNSLD_PASSWORD;
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // stream
  if(!Firebase.RTDB.beginStream(&fbdo_ledStatus, LED_STATUS_PATH)){
    Serial.printf("Stream Error: %s", fbdo_ledStatus.errorReason().c_str());
  }
  if(!Firebase.RTDB.beginStream(&fbdo_ledBrightness, LED_BRIGHTNESS_PATH)){
    Serial.printf("Stream Error: %s", fbdo_ledBrightness.errorReason().c_str());
  }
  if(!Firebase.RTDB.beginStream(&fbdo_ledColorValue, LED_COLOR_VALUE_PATH)){
    Serial.printf("Stream Error: %s", fbdo_ledColorValue.errorReason().c_str());
  }
  if(!Firebase.RTDB.beginStream(&fbdo_curtainState, CURTAIN_STATE_PATH)){
    Serial.printf("Stream Error: %s", fbdo_curtainState.errorReason().c_str());
  }
  if(!Firebase.RTDB.beginStream(&fbdo_curtainCloseDuration, CURTAIN_CLOSE_DURATION_PATH)){
    Serial.printf("Stream Error: %s", fbdo_curtainCloseDuration.errorReason().c_str());
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

void changeCurtainState(int state){
  curtainState = state;
  Serial.print("Curtain State: ");
  Serial.println(curtainState);
}

void getCurtainCloseDuration(int seconds){
  curtainCloseDuration = seconds;
  for(int i = 0; i < curtainCloseDuration; i++){
    Serial.print(curtainCloseDuration - i);
    Serial.println(" seconds remaining.");
    delay(1000);
  }
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
    if(!Firebase.RTDB.readStream(&fbdo_curtainState)){
      Serial.printf("Stream Error: %s", fbdo_curtainState.errorReason().c_str());
    }
    if(fbdo_curtainState.streamAvailable()){
      changeCurtainState(fbdo_curtainState.intData());
    }
    if(!Firebase.RTDB.readStream(&fbdo_curtainCloseDuration)){
      Serial.printf("Stream Error: %s", fbdo_curtainCloseDuration.errorReason().c_str());
    }
    if(fbdo_curtainCloseDuration.streamAvailable()){
      getCurtainCloseDuration(fbdo_curtainCloseDuration.intData());
    }
  }
}
