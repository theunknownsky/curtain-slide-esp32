#include <Arduino.h>
#include <WiFiManager.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define API_KEY "AIzaSyB3ekwAq5EGkH_MsXUCa1R3NxIH7KTK6zk"
#define DB_URL "https://curtainslide-test-default-rtdb.asia-southeast1.firebasedatabase.app/"

#define LED1 21

bool isWifiConnected;

FirebaseData fbdo, fbdo_1, fbdo_2;
FirebaseAuth auth;
FirebaseConfig config;

String ctnsld_email = "test@ctnsld.co";
String ctnsld_pword = "12345678";
String ctnsld_userid = "pOMaE87MRrZD3lg2kyO306XzndR2";

String ledStatusPath = "users/pOMaE87MRrZD3lg2kyO306XzndR2/ledInfo/ledStatus";

unsigned long sendDataPrevMillis = 0;
bool ledStatus;

void setup() {
  WiFiManager wm;
  // to make sure every run, the wifi is reset
  // wm.resetSettings();
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(LED1, OUTPUT);
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
  if(!Firebase.RTDB.beginStream(&fbdo, ledStatusPath)){
    Serial.printf("Stream Error: %s", fbdo.errorReason().c_str());
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  if(Firebase.ready()){
    if(!Firebase.RTDB.readStream(&fbdo)){
      Serial.printf("Stream Error: %s", fbdo.errorReason().c_str());
    }
    if(fbdo.streamAvailable()){
      ledStatus = fbdo.boolData();
      digitalWrite(LED1, ledStatus);
    }
  }
}