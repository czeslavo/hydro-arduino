#include <SPI.h>
#include <WiFiNINA.h>
#include <PubSubClient.h>
#include "arduino_secrets.h" 

char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)

WiFiClient wifiClient;
PubSubClient pubsubClient("192.168.8.23", 1883, wifiClient);
int wifiStatus = WL_IDLE_STATUS;

void setupSerial() {
  Serial.begin(9600);
  while (!Serial); 
}

void setupWifi() {  
  while (wifiStatus != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    
    wifiStatus = WiFi.begin(ssid, pass);

    // wait for connection:
    delay(10000);
  }

  Serial.print("Connected to WIFI, IP Address: ");
  Serial.println(WiFi.localIP());
}

void setupWifiIfNeeded() {
  wifiStatus = WiFi.status();
  if (wifiStatus != WL_CONNECTED) {
    Serial.print("Status ");
    Serial.print(wifiStatus);
    Serial.println(", reconnecting");
    setupWifi();
  }
}

void setupPubsub() {
  bool connected = pubsubClient.connect("ph-meter");
  if (connected) {
    Serial.println("Connected to broker");
  }
}

void setupPubsubIfNeeded() {
  if (!pubsubClient.loop()) {
    Serial.print("Pubsub status: ");
    Serial.println(pubsubClient.state());
    setupPubsub();
  }
}

void setup() {


  setupWifi();
  setupPubsub();
}

void reportPh() {
  if (pubsubClient.publish("hydro/ph", "5")) {
    Serial.println("Published to hydro/ph");
  } else {
    Serial.println("Failed to publish message");
  }
}

void loop() {
  setupWifiIfNeeded();
  setupPubsubIfNeeded();
  reportPh();
  
  delay(10000);
}
