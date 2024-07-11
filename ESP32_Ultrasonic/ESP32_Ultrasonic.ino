#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include "Conf.h"  // configuration file for wifi and thingspeak credentials

#define echoPin 2
#define trigPin 4

const int Max_DISTANCE = 100; //maximum distance(height of the bin) in cm
const int MAX_RETRIES = 5; // specifying maximum number of retries for HTTP requests
const int RETRY_DELAY = 5000; //time between next retry in milliseconds

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
  // Triger ultrasonic sensor
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

// read the echo pin
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;

  //handling error sensor readings
  if (distance <= 0 || distance > Max_DISTANCE){
    Serial.println("Error: Invalid sensor reading");
    delay(10000); // wait 10 seconds before retrying
    return;
  }

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

 // send data to thingspeak if data is connected
  if (WiFi.status() == WL_CONNECTED){
    HTTPClient http;
    String url = String(SERVER) + "?api_key=" + WRITE_APIKEY +  "$field1=" + String(fillLevel);

    int retries = 0;
    int httpResponseCode = -1;
    while(retries < MAX_RETRIES){
      http.begin(url);
    int httpResponseCode = http.GET();
    if (httpResponseCode > 0){
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);

      break;
    }
    else{
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
      retries++;
      Serial.print("Retrying....(");
      Serial.print(retries);
      Serial.println(")");
      delay(RETRY_DELAY); // wait before retrying

    }
    http.end();
  }
  if (httpResponseCode <= 0){
    Serial.println("failed to send data after retries");
  } else{
    Serial.println("Wifi Disconnected");
  
  }
  delay(15000); // send data every 15 seconds 
  }
}
