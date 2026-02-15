#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

//PIN CONFIG 
#define PIR_PIN 13
#define LDR_PIN 10
#define DHT_PIN 4

#define LIGHT_RELAY 2
#define FAN_RELAY 18
#define AC_RELAY 19

#define DHTTYPE DHT22
DHT dht(DHT_PIN, DHTTYPE);

//WIFI 
const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* mqtt_server = "demo.thingsboard.io";
const char* accessToken = "ve0s5kv3y9ux5rxgzgx4";

WiFiClient espClient;
PubSubClient client(espClient);

//ENERGY VARIABLES 
unsigned long lightOnTime = 0;
unsigned long fanOnTime = 0;
unsigned long acOnTime = 0;

unsigned long lightStart = 0;
unsigned long fanStart = 0;
unsigned long acStart = 0;

bool lightState = false;
bool fanState = false;
bool acState = false;

const float lightPower = 160.0;
const float fanPower = 75.0;
const float acPower = 1500.0;
const float costPerUnit = 7.0;

//WIFI
void connectWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");
}

//MQTT
void reconnect() {
  while (!client.connected()) {
    Serial.print("Connecting MQTT...");
    if (client.connect("ESP32_Client", accessToken, NULL)) {
      Serial.println("Connected!");
    } else {
      Serial.print("Failed rc=");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

//SETUP 
void setup() {
  Serial.begin(115200);

  pinMode(PIR_PIN, INPUT);
  pinMode(LDR_PIN, INPUT);

  pinMode(LIGHT_RELAY, OUTPUT);
  pinMode(FAN_RELAY, OUTPUT);
  pinMode(AC_RELAY, OUTPUT);

  digitalWrite(LIGHT_RELAY, LOW);
  digitalWrite(FAN_RELAY, LOW);
  digitalWrite(AC_RELAY, LOW);

  dht.begin();

  connectWiFi();
  client.setServer(mqtt_server, 1883);

  Serial.println("System Ready");
}

// LOOP 
void loop() {

  if (!client.connected()) reconnect();
  client.loop();

  bool motion = digitalRead(PIR_PIN);
  int ldrValue = analogRead(LDR_PIN);

  //Adjust threshold if needed
  bool dark = (ldrValue < 2000);

  float temperature = dht.readTemperature();

  if (isnan(temperature)) {
    Serial.println("DHT Error");
    return;
  }

  Serial.print("LDR: ");
  Serial.print(ldrValue);
  Serial.print(" | Temp: ");
  Serial.println(temperature);

  unsigned long currentTime = millis();

  bool newLight = false;
  bool newFan = false;
  bool newAC = false;

  //LOGIC
  if (motion) {

    if (!dark) {   

      if (temperature > 41) {
        newAC = true;
      }
      else if (temperature > 37) {
        newFan = true;
      }

    } else {       

      newLight = true;

      if (temperature > 41) {
        newAC = true;
      }
      else if (temperature > 37) {
        newFan = true;
      }
    }
  }
  

  // RELAY + ENERGY 

  // LIGHT
  if (newLight && !lightState) {
    digitalWrite(LIGHT_RELAY, HIGH);
    lightStart = currentTime;
    lightState = true;
  }
  if (!newLight && lightState) {
    digitalWrite(LIGHT_RELAY, LOW);
    lightOnTime += currentTime - lightStart;
    lightState = false;
  }

  // FAN
  if (newFan && !fanState) {
    digitalWrite(FAN_RELAY, HIGH);
    fanStart = currentTime;
    fanState = true;
  }
  if (!newFan && fanState) {
    digitalWrite(FAN_RELAY, LOW);
    fanOnTime += currentTime - fanStart;
    fanState = false;
  }

  // AC
  if (newAC && !acState) {
    digitalWrite(AC_RELAY, HIGH);
    acStart = currentTime;
    acState = true;
  }
  if (!newAC && acState) {
    digitalWrite(AC_RELAY, LOW);
    acOnTime += currentTime - acStart;
    acState = false;
  }

  //Running energy 
  unsigned long tempLightTime = lightOnTime;
  unsigned long tempFanTime = fanOnTime;
  unsigned long tempAcTime = acOnTime;

  if (lightState) tempLightTime += currentTime - lightStart;
  if (fanState)   tempFanTime   += currentTime - fanStart;
  if (acState)    tempAcTime    += currentTime - acStart;

  float lightEnergy = (tempLightTime / 3600000.0) * lightPower / 1000.0;
  float fanEnergy   = (tempFanTime / 3600000.0) * fanPower / 1000.0;
  float acEnergy    = (tempAcTime / 3600000.0) * acPower / 1000.0;

  float totalEnergy = lightEnergy + fanEnergy + acEnergy;
  float totalCost = totalEnergy * costPerUnit;

  //MQTT JSON 
  String payload = "{";
  payload += "\"motion\":" + String(motion) + ",";
  payload += "\"dark\":" + String(dark) + ",";
  payload += "\"temperature\":" + String(temperature) + ",";
  payload += "\"light\":" + String(lightState) + ",";
  payload += "\"fan\":" + String(fanState) + ",";
  payload += "\"ac\":" + String(acState) + ",";
  payload += "\"energy_kWh\":" + String(totalEnergy,4) + ",";
  payload += "\"cost\":" + String(totalCost,2);
  payload += "}";

  client.publish("v1/devices/me/telemetry", payload.c_str());

  Serial.println(payload);

  delay(2000);
}