#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include "Conf.h"  // configuration file for wifi and thingspeak credentials

#define echoPin 2
#define trigPin 4

const int Max_DISTANCE = 100; //maximum distance(height of the bin) in cm

long duration, distance;
int fillLevel;


void setup() {
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Wifi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

}

void loop() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  //calculate bin as percentage in relation to distance
  fillLevel = 100 - ((distance * 100) / Max_DISTANCE);
  if (fillLevel < 0) fillLevel = 0;
  if (fillLevel > 100) fillLevel = 100;
  Serial.print("Fill Level: ");
  Serial.print(fillLevel);
  Serial.println("%");

  if (WiFi.status() == WL_CONNECTED){
    HTTPClient http;
    String url = String(SERVER) + "?api_key=" + WRITE_APIKEY +  "$field1=" + String(fillLevel);
    http.begin(url);
    int httpResponseCode = http.GET();
    if (httpResponseCode > 0){
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);

    }else{
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);

    }
    http.end();
  }else{
    Serial.println("Wifi Disconnected");
  
  }
  delay(15000); // send data every 15 seconds 

}
