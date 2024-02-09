#if defined(ESP32)
#include <WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <FirebaseESP32.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#endif
#include <HTTPClient.h>
#include <time.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
#define WIFI_SSID "IoT"      // Router name
#define WIFI_PASSWORD "12345678"  // Router password
#define API_KEY "AIzaSyBVJjAe6nAx5xizIHf2TBmJrWXnew6SaDc"
#define DATABASE_URL "iot-project-6fecf-default-rtdb.asia-southeast1.firebasedatabase.app" 
#define USER_EMAIL "abcd@gmail.com"
#define USER_PASSWORD "12345678"


float calibration = 21.52; //change this value to calibrate
const int analogInPin = 39; 
int sensorValue = 0; 
unsigned long int avgValue; 
float b;
int buf[10],temp;

#define pHpin 36
#define TdsSensorPin 39


FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

OneWire oneWire(4);
DallasTemperature sensors(&oneWire);







//===========For TDS==============



#define VREF 3.3              // analog reference voltage(Volt) of the ADC
#define SCOUNT  30            // sum of sample point

int analogBuffer[SCOUNT];     // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0;
int copyIndex = 0;

float averageVoltage = 0;
float tdsValue = 0;
float temperatureC = 25;       // current temperature for compensation




int getMedianNum(int bArray[], int iFilterLen){
  int bTab[iFilterLen];
  for (byte i = 0; i<iFilterLen; i++)
  bTab[i] = bArray[i];
  int i, j, bTemp;
  for (j = 0; j < iFilterLen - 1; j++) {
    for (i = 0; i < iFilterLen - j - 1; i++) {
      if (bTab[i] > bTab[i + 1]) {
        bTemp = bTab[i];
        bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
      }
    }
  }
  if ((iFilterLen & 1) > 0){
    bTemp = bTab[(iFilterLen - 1) / 2];
  }
  else {
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
  }
  return bTemp;
}











//=================================





unsigned long sendDataPrevMillis = 0;
unsigned long count = 0;

const char *timeApiUrl = "http://worldtimeapi.org/api/timezone/Asia/Dhaka";  // Time API endpoint
unsigned long previousMillis = 0;
const long interval = 2000;  // 2 minutes in milliseconds


String setting[3],setting2[3];



void setup()
{

  Serial.begin(9600);


  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
  config.api_key = API_KEY;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.database_url = DATABASE_URL;
  config.token_status_callback = tokenStatusCallback;



  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  Firebase.setDoubleDigits(5);
  pinMode(36,INPUT);  //Sensor1
  pinMode(39,INPUT);  //Sensor2
  pinMode(34,INPUT);  //Sensor3
  pinMode(35,INPUT);  //Sensor4
  pinMode(2,OUTPUT); //Pump1
  pinMode(5,OUTPUT); //Pump2
  configTime(6 * 3600, 0, "pool.ntp.org", "time.nist.gov");


  sensors.begin();
  pinMode(TdsSensorPin,INPUT);

  FirebaseData fbdo;
  if (Firebase.getString(fbdo, "/Setting")) {
    if (fbdo.dataType() == "string") {
      String firebaseValue = fbdo.stringData();
      Serial.println("Firebase value: " + firebaseValue);

 
      int index = 0;
      int lastIndex = -1;


      setting[0] = firebaseValue.substring(0, 1);
      setting[1] = firebaseValue.substring(2, 3);
      setting[2] = firebaseValue.substring(4, firebaseValue.length());
      
      


      Serial.println("Values stored in the array:");
      for (int i = 0; i < 3; i++) {
        Serial.println(setting[i]);
      }
    }
  } else {
    Serial.println(fbdo.errorReason());
  }



    if (Firebase.getString(fbdo, "/Setting2")) {
    if (fbdo.dataType() == "string") {
      String firebaseValue = fbdo.stringData();
      Serial.println("Firebase value: " + firebaseValue);

 
      int index = 0;
      int lastIndex = -1;


      setting2[0] = firebaseValue.substring(0, 1);
      setting2[1] = firebaseValue.substring(2, 3);
      setting2[2] = firebaseValue.substring(4, firebaseValue.length());
      
      


      Serial.println("Values stored in the array:");
      for (int i = 0; i < 3; i++) {
        Serial.println(setting2[i]);
      }
    }
  } else {
    Serial.println(fbdo.errorReason());
  }
  
}



int del=1000;
float pH=3.3;


float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}



void loop()
{


for(int i=0;i<200;i++){
    static unsigned long analogSampleTimepoint = millis();
  if(millis()-analogSampleTimepoint > 40U){     //every 40 milliseconds,read the analog value from the ADC
    analogSampleTimepoint = millis();
    analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin);    //read the analog value and store into the buffer
    analogBufferIndex++;
    if(analogBufferIndex == SCOUNT){ 
      analogBufferIndex = 0;
    }
  }   
  
  static unsigned long printTimepoint = millis();
  if(millis()-printTimepoint > 800U){
    printTimepoint = millis();
    for(copyIndex=0; copyIndex<SCOUNT; copyIndex++){
      analogBufferTemp[copyIndex] = analogBuffer[copyIndex];
      averageVoltage = getMedianNum(analogBufferTemp,SCOUNT) * (float)VREF / 4096.0;
      float compensationCoefficient = 1.0+0.02*(temperatureC-25.0);
      float compensationVoltage=averageVoltage/compensationCoefficient;
      tdsValue=(133.42*compensationVoltage*compensationVoltage*compensationVoltage - 255.86*compensationVoltage*compensationVoltage + 857.39*compensationVoltage)*0.5;

    }
  }

}















//=============For pH Calculation===============

 for(int i=0;i<10;i++) 
 { 
 buf[i]=analogRead(pHpin);
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
 float pHVol=(float)avgValue*3.33/4096/6;
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



 //=======================================================
























  


 unsigned long currentMillis = millis();

  sensors.requestTemperatures(); 
  temperatureC = sensors.getTempCByIndex(0);
  
  String genText=String(temperatureC)+":"+String(tdsValue)+":"+String(phValue)+":"+String(mapfloat(analogRead(34),0.00,4096.00,1.0,3.0));

  
  delay(del);

    if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    time_t now = time(nullptr);
    if (now != 0) {
      struct tm *timeInfo;
      timeInfo = localtime(&now);
      char dateTimeString[20];
      sprintf(dateTimeString, "%04d'%02d'%02d'%02d'%02d'%02d",
              timeInfo->tm_year + 1900, timeInfo->tm_mon + 1, timeInfo->tm_mday,
              timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec);
      Serial.println(dateTimeString);
       Serial.printf("Firebase data: %s\n", Firebase.setString(fbdo, "/Device/"+String(dateTimeString), genText) ? String(genText) : fbdo.errorReason().c_str());
    } else {
      Serial.println("Failed to obtain time");
    }
 }



  float value = setting[2].toFloat();


  if(setting[0]=="1"){ //For pH

    if(setting[1]=="1") //greater then
      digitalWrite(2,value<phValue);
    else
      digitalWrite(2,value>phValue);
  
    
  }else{  //For temperature

    if(setting[1]=="1") //greater then
      digitalWrite(2,value<temperatureC);
    else
      digitalWrite(2,value>temperatureC);
    
  }


  value=setting2[2].toFloat();

  
  if(setting2[0]=="1"){ //For TDS

    if(setting2[1]=="1") //greater then
      digitalWrite(5,value<tdsValue);
    else
      digitalWrite(5,value>tdsValue);
  
    
  }else{  //For EC

    if(setting2[1]=="1") //greater then
      digitalWrite(5,value<mapfloat(analogRead(A0),0.00,4096.00,1.5,2.5));
    else
      digitalWrite(5,value>mapfloat(analogRead(A0),0.00,4096.00,1.5,2.5));
    
  }
  
// 
//  if(setting[1]=="1") { //Greaten than
//    if(setting[0]=="1") //pH
//      digitalWrite(2,value<pH);
//    else
//      digitalWrite(2,value<temperatureC);
//  }else{ //less than
//    if(setting[0]=="1") //pH
//      digitalWrite(2,value>pH);
//    else
//      digitalWrite(2,value>temperatureC);
//  }
 

}
