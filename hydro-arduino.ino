#include <SPI.h>
#include <WiFiNINA.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "DFRobot_PH.h"
#include <EEPROM.h>
#include <Adafruit_NeoPixel.h>

#include "pixels.h"     
#include "secrets.h" 

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

#define PH_PIN A1
#define ONE_WIRE_BUS 2
#define DIODE_PIN 4
#define DIODES_COUNT 16

DFRobot_PH ph;

OneWire oneWire(ONE_WIRE_BUS); 
DallasTemperature waterTemperatureSensors(&oneWire);

WiFiClient wifiClient;
PubSubClient pubsubClient("192.168.8.23", 1883, wifiClient);

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(DIODES_COUNT, DIODE_PIN, NEO_GRB + NEO_KHZ800);

int wifiStatus = WL_IDLE_STATUS;

void setupSerial() {
  Serial.begin(9600);
  while (!Serial); 
}

void setupWaterTemperatureSensors() {
  waterTemperatureSensors.begin();
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

void setupPixels() {
  pixels.begin(); 
}

void setup() {
  setupSerial();
  setupWaterTemperatureSensors();

  setupWifi();
  setupPubsub();
  setupPixels();
}

void phMeterLoop()
{
  static unsigned long timepoint = millis();
  float phVoltage, temperature = 25;
  
  if(millis()-timepoint>1000U) {                 
      timepoint = millis();
      
      temperature = readWaterTemperature();    
      
      phVoltage = analogRead(PH_PIN) / 1024.0 * 5000;
      float phValue = ph.readPH(phVoltage, temperature);

      Serial.print("Read pH: ");
      Serial.print(phValue);
      Serial.print(" with temp: ");
      Serial.println(temperature);
      
      reportPhWithTemperature(phValue, temperature);
      updatePhPixels(&pixels, phValue);
      updateTemperaturePixels(&pixels, temperature);
    }
    
    
  ph.calibration(phVoltage, temperature);
}

void reportPhWithTemperature(float phValue, float temperature) {
  char buffer[100]; 
  Serial.println(phValue);
  sprintf(
    buffer, 
    "{\"ph\":  %d.%02d, \"temperature\": %d.%02d}", 
    (int)phValue, (int)(phValue*100)%100,
    (int)temperature, (int)(temperature*100)%100
  );
  
  if (!pubsubClient.publish("hydro/wiadra", buffer)) {
    Serial.println("Failed to report pH value");
    return;
  }

  Serial.print("Reported pH value: ");
  Serial.println(buffer);
}

float readWaterTemperature() {
  waterTemperatureSensors.requestTemperatures();
  return waterTemperatureSensors.getTempCByIndex(0); 
}

void loop() {
  setupWifiIfNeeded();
  setupPubsubIfNeeded();
  
  phMeterLoop();
  pixels.show();
}
