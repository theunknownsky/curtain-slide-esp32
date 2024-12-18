#include <Arduino.h>
#include <WiFiManager.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include <credentials.h>
#include <FastLED.h>

#define SWITCH_CLOSE 22
#define SWITCH_OPEN 23

#define NUM_LEDS 30 // TO BE CHANGED UPON BUILD
#define DATA_PIN 17 // TO BE CHANGED UPON BUILD (TO 25???)

// #define ENA 14
#define MTRA1 27
#define MTRA2 26

#define MTRB1 25
#define MTRB2 33
// #define ENB 32

FirebaseData fbdo_ledStatus, fbdo_ledBrightness, fbdo_ledColorValue, fbdo_curtainState, fbdo_curtainCloseDuration, fbdo_isCurtainClosed, fbdo_isCurtainOpened;
FirebaseAuth auth;
FirebaseConfig config;

bool isWifiConnected;
bool ledStatus;
int ledBrightness;
int actualLedBrightness;
int red;
int green;
int blue;
int curtainState = 1;
int curtainCloseDuration;
bool isCurtainOpened = false;
bool isCurtainClosed = false;

CRGB leds[NUM_LEDS]; // setyup WS2812B

const int freq = 30000;
const int pwmChannelA = 0; // PWM channel for Motor A
const int pwmChannelB = 1; // PWM channel for Motor B
const int resolution = 8;
int dutyCycleA = 255;
int dutyCycleB = 255;

void setup() {
  WiFiManager wm;
  // to make sure every run, the wifi is reset
  // wm.resetSettings();
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(DATA_PIN, OUTPUT);
  // pinMode(ENA, OUTPUT);
  pinMode(MTRA1, OUTPUT);
  pinMode(MTRA2, OUTPUT);
  pinMode(MTRB1, OUTPUT);
  pinMode(MTRB2, OUTPUT);
  // pinMode(ENB, OUTPUT);
  pinMode(SWITCH_CLOSE, INPUT_PULLUP);
  pinMode(SWITCH_OPEN, INPUT_PULLUP);
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS); 

  // Configure LEDC PWM for Motor A
  // ledcSetup(pwmChannelA, freq, resolution);
  // ledcAttachPin(ENA, pwmChannelA);

  // Configure LEDC PWM for Motor B
  // ledcSetup(pwmChannelB, freq, resolution);
  // ledcAttachPin(ENB, pwmChannelB);

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
  if(!Firebase.RTDB.beginStream(&fbdo_isCurtainClosed, IS_CURTAIN_CLOSED_PATH)){
    Serial.printf("Stream Error: %s", fbdo_isCurtainClosed.errorReason().c_str());
  }
  if(!Firebase.RTDB.beginStream(&fbdo_isCurtainOpened, IS_CURTAIN_OPENED_PATH)){
    Serial.printf("Stream Error: %s", fbdo_isCurtainOpened.errorReason().c_str());
  }
}

void turnOnWS2812B(){
  FastLED.setBrightness(actualLedBrightness);
  for(int i = 0; i < NUM_LEDS; i++){
    leds[i] = CRGB(red, green, blue);
  }
  FastLED.show();
}

void turnOffWS2812B(){
  FastLED.setBrightness(0);
  for(int i = 0; i < NUM_LEDS; i++){
    leds[i] = CRGB::Black;
  }
  FastLED.show();
}

void switchLEDStatus(bool status){
  ledStatus = status;
  // digitalWrite(LED1, ledStatus);
  Serial.print("LED Status: ");
  Serial.println(ledStatus);
  // Switch LED/RGB Strip to on or off
  if(ledStatus){
    turnOnWS2812B();
  } else {
    turnOffWS2812B();
  }
}

void changeLEDBrightness(int brightness){
  ledBrightness = brightness;
  Serial.print("LED Brightness: ");
  Serial.println(ledBrightness);
  // Change LED/RGB Strip brightness 
  actualLedBrightness = ledBrightness * 5; // 'A' value for ARGB goes from 0 to 255 (50 would be the max for this)
  turnOnWS2812B();
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
  // Change LED/RGB Strip color (most likely along with 'A' value of ARGB)
  turnOnWS2812B();
}

void closeCurtain(){
  // ledcWrite(pwmChannelA, dutyCycleA);
  digitalWrite(MTRA1, HIGH);
  digitalWrite(MTRA2, LOW);
  // ledcWrite(pwmChannelB, dutyCycleB);
  digitalWrite(MTRB1, HIGH);
  digitalWrite(MTRB2, LOW);
}

void openCurtain(){
  // ledcWrite(pwmChannelA, dutyCycleA);
  digitalWrite(MTRA1, LOW);
  digitalWrite(MTRA2, HIGH);
  // ledcWrite(pwmChannelB, dutyCycleB);
  digitalWrite(MTRB1, LOW);
  digitalWrite(MTRB2, HIGH);
}

void stopCurtain(){
  // ledcWrite(pwmChannelA, dutyCycleA);
  digitalWrite(MTRA1, LOW);
  digitalWrite(MTRA2, LOW);
  // ledcWrite(pwmChannelB, dutyCycleB);
  digitalWrite(MTRB1, LOW);
  digitalWrite(MTRB2, LOW);
}

void changeCurtainState(int state){
  curtainState = state;
  Serial.print("Curtain State: ");
  Serial.println(curtainState);
  if(curtainState == 0){
    closeCurtain();
  } else if (curtainState == 2){
    openCurtain();
  } else if (curtainState == 1){
    stopCurtain();
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  if(Firebase.ready()){ // making sure Firebase is ready before calling any firebase function
    Serial.print("Firebase RTDB Ready. ");
    if(!Firebase.RTDB.readStream(&fbdo_ledStatus)){ // reads ledStatus
      Serial.printf("Stream Error: %s", fbdo_ledStatus.errorReason().c_str());
    }
    if(fbdo_ledStatus.streamAvailable()){ // checks if ledStatus is updated
      switchLEDStatus(fbdo_ledStatus.boolData());
    }
    if (ledStatus){ // checks if ledStatus is on, the following functions only gets called if ledStatus is on
      if(!Firebase.RTDB.readStream(&fbdo_ledBrightness)){ // reads ledBrightness
        Serial.printf("Stream Error: %s", fbdo_ledBrightness.errorReason().c_str());
      }
      if(fbdo_ledBrightness.streamAvailable()){ // checks if ledBrightness is updated
        changeLEDBrightness(fbdo_ledBrightness.intData());
      }
      if(!Firebase.RTDB.readStream(&fbdo_ledColorValue)){ // reads ledColorValue (hexcode)
        Serial.printf("Stream Error: %s", fbdo_ledColorValue.errorReason().c_str());
      }
      if(fbdo_ledColorValue.streamAvailable()){ // checks if ledColorValue is updated
        changeLEDColor(fbdo_ledColorValue.stringData());
      }
    }

    if(!Firebase.RTDB.readStream(&fbdo_curtainState)){ // reads curtainState
      Serial.printf("Stream Error: %s", fbdo_curtainState.errorReason().c_str());
    }
    if(fbdo_curtainState.streamAvailable()){ // checks if curtainState is updated
      changeCurtainState(fbdo_curtainState.intData());
    }
    if(!Firebase.RTDB.readStream(&fbdo_isCurtainClosed)){ // reads isCurtainClosed
      Serial.printf("Stream Error: %s", fbdo_isCurtainClosed.errorReason().c_str());
    }
    if(fbdo_isCurtainClosed.streamAvailable()){ // checks if isCurtainClosed is updated
      isCurtainClosed = fbdo_isCurtainClosed.boolData();
    }
    if(!Firebase.RTDB.readStream(&fbdo_isCurtainOpened)){ // reads isCurtainOpened
      Serial.printf("Stream Error: %s", fbdo_isCurtainOpened.errorReason().c_str());
    }
    if(fbdo_isCurtainOpened.streamAvailable()){ // checks if isCurtainOpened is updated
      isCurtainOpened = fbdo_isCurtainOpened.boolData();
    }

    if(isCurtainClosed != !digitalRead(SWITCH_CLOSE)){ // following actions only gets called if the digitalRead(SWITCH_CLOSE) gives a different value 
      isCurtainClosed = !digitalRead(SWITCH_CLOSE);
      if(!Firebase.RTDB.setBool(&fbdo_isCurtainClosed, IS_CURTAIN_CLOSED_PATH, isCurtainClosed)){ // changes isCurtainClosed in Firebase RTDB
        Serial.printf("Stream Error: %s", fbdo_isCurtainClosed.errorReason().c_str());
      }
      Serial.println("Curtain Closed Status Changing...");
      Serial.print("Curtain Closed: ");
      Serial.println(isCurtainClosed);
      if(curtainState == 0){
        changeCurtainState(1);
        if(!Firebase.RTDB.setInt(&fbdo_curtainState, CURTAIN_STATE_PATH, 1)){ // resets curtainState (to 1 - not moving) in Firebase RTDB
          Serial.printf("Stream Error: %s", fbdo_curtainState.errorReason().c_str());
        }
      }
    }
    if(isCurtainOpened != !digitalRead(SWITCH_OPEN)){ // following actions only gets called if the digitalRead(SWITCH_OPEN) gives a different value 
      isCurtainOpened = !digitalRead(SWITCH_OPEN);
      if(!Firebase.RTDB.setBool(&fbdo_isCurtainOpened, IS_CURTAIN_OPENED_PATH, isCurtainOpened)){ // changes isCurtainOpened in Firebase RTDB
        Serial.printf("Stream Error: %s", fbdo_isCurtainOpened.errorReason().c_str());
      }
      Serial.println("Curtain Opened Status Changing...");
      Serial.print("Curtain Opened: ");
      Serial.println(isCurtainOpened);
      if(curtainState == 2){
        changeCurtainState(1);
        if(!Firebase.RTDB.setInt(&fbdo_curtainState, CURTAIN_STATE_PATH, 1)){ // resets curtainState (to 1 - not moving) in Firebase RTDB
          Serial.printf("Stream Error: %s", fbdo_curtainState.errorReason().c_str());
        }
      }
    }
    delay(1000);
  } else {
    Serial.println("Firebase RTDB not ready. ");
    delay(1000);
  }
}
