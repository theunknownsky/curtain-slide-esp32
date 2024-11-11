#include <Arduino.h>
#include <WiFiManager.h>

#define LED1 17

bool res;

void resetWifi(){
  WiFiManager wm;
  wm.resetSettings();
}

bool initWifi(){
  WiFiManager wm;
  bool res;
  res = wm.autoConnect("CTNSLD-69", "testPass");
  return res;
}

void setup() {

  resetWifi();
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(LED1, OUTPUT);

  WiFiManager wm;
  res = wm.autoConnect("CTNSLD-69", "testPass");

  if(!res){
    Serial.println("Failed to connect.");
    ESP.restart();
  } else {
    Serial.println("You are now connected!");
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  if(res){
    digitalWrite(LED1, 1);
  }
  
}