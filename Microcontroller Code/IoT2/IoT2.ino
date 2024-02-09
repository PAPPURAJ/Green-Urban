#include <FirebaseESP8266.h>
#include <ESP8266WiFi.h>



#define FIREBASE_HOST "iotproject-7df76-default-rtdb.firebaseio.com"  // Database link
#define FIREBASE_AUTH "LSJLWsMtR7n52UUOlYe0PCUUC9y0vRTy1DssMCwx"    // Database secret

#define WIFI_SSID "IoT"      // Router name
#define WIFI_PASSWORD "12345678"  // Router password

#include <Wire.h> 

float calibration = 22.42; //change this value to calibrate
const int analogInPin = A0; 
int sensorValue = 0; 
unsigned long int avgValue; 
float b;
int buf[10],temp;

FirebaseData firebaseData;

void initFirebase() {
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}

void sendToFirebase(String path, String value) {
  Firebase.setString(firebaseData, path, value);
}

void setup() {
  pinMode(13, OUTPUT);
  pinMode(D5,OUTPUT);
  Serial.begin(9600);

  // Connect to Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nConnected to Wi-Fi");

  initFirebase();
}

void loop() {
for(int i=0;i<10;i++) 
 { 
 buf[i]=analogRead(analogInPin);
 delay(30);
 }
 for(int i=0;i<9;i++)
 {
 for(int j=i+1;j<10;j++)
 {
 if(buf[i]>buf[j])
 {
 temp=buf[i];
 buf[i]=buf[j];
 buf[j]=temp;
 }
 }
   }
 avgValue=0;
 for(int i=2;i<8;i++)
 avgValue+=buf[i];
 float pHVol=(float)avgValue*3.3/1024/6;
 float phValue = -5.70 * pHVol + calibration;
 Serial.println(phValue);
 Serial.println("pH: ");
 Serial.println(phValue);
 if (phValue < 4) {
  Serial.println("Very acidic");
 }
 else if (phValue >= 4 && phValue < 5) {
  Serial.println("Acidic");
 }
 else if (phValue >= 5 && phValue < 7) {
  Serial.println("Acidic-ish");
 }
 else if (phValue >= 7 && phValue < 8) {
  Serial.println("Neutral");
 }
 else if (phValue >= 8 && phValue < 10) {
  Serial.println("Alkaline-ish");
 }
 else if (phValue >= 10 && phValue < 11) {
  Serial.println("Alkaline");
 }
 else if (phValue >= 11) {
  Serial.println("Very alkaline");
 }


  digitalWrite(D5,phValue>=5.5 && phValue<=6.5);
  sendToFirebase("/Sensors/pH", String(phValue));


  delay(1000);
}
