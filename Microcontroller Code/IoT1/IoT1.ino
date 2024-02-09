#include "FirebaseESP8266.h"
#include <ESP8266WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define FIREBASE_HOST "iotproject-7df76-default-rtdb.firebaseio.com"  // Database link
#define FIREBASE_AUTH "LSJLWsMtR7n52UUOlYe0PCUUC9y0vRTy1DssMCwx"    // Database secret

#define WIFI_SSID "IoT"      // Router name
#define WIFI_PASSWORD "12345678"  // Router password

#define TDS_SENSOR_PIN A0  // Analog pin for TDS sensor
#define DS18B20_SENSOR_PIN D1  // Digital pin for DS18B20 sensor

FirebaseData firebaseData;

// Setup OneWire and DallasTemperature
OneWire oneWire(DS18B20_SENSOR_PIN);
DallasTemperature sensors(&oneWire);



//==========For TDS==============




void initFire() {
  pinMode(D4, OUTPUT);
  Serial.begin(9600);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(D4, 0);
    Serial.print(".");
    delay(200);
    digitalWrite(D4, 1);
    Serial.print(".");
    delay(200);
  }

  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
}

void sendToFirebase(String path, String value) {
  Firebase.setString(firebaseData, path, value);
}

float readTemperature() {
  // Request temperature from DS18B20 sensor
  sensors.requestTemperatures();
  // Read temperature in Celsius
  float temperatureC = sensors.getTempCByIndex(0);
  return temperatureC;
}

void setup() {
  pinMode(D5, INPUT);
  pinMode(A0,INPUT);
  Serial.begin(9600);
  initFire();
  sensors.begin();
  delay(500);
}


float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void loop() {

  sendToFirebase("/Sensors/EC", String(mapfloat(analogRead(A0),0.00,1023.00,1.5,2.5))+"mS/cm");
  sendToFirebase("/Sensors/Temperature", String(readTemperature())+"°c");

  delay(500); // Adjust delay as needed
}
