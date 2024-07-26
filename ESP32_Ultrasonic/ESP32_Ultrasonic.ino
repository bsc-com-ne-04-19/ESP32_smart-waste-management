#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include "Conf.h"  // Configuration file for Wi-Fi and ThingSpeak credentials

#define echoPin 5
#define trigPin 18

const int Max_DISTANCE = 20; // Maximum distance (height of the bin) in cm
const int Max_RETRIES = 3; // Maximum number of retries for HTTP requests
const int RETRY_DELAY = 5000; // Delay between retries in milliseconds

long duration, distance;
int fillLevel;

void setup() {
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Connect to Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Trigger the ultrasonic sensor
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Read the echo pin
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;

  // Error handling for sensor reading
  if (distance <= 0) {
    Serial.println("Error: Invalid distance reading");
    delay(15000); // Wait before retrying
    return;
  }

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  // Calculate fill level as a percentage
  if (distance >= Max_DISTANCE) {
    fillLevel = 100;
  } else {
    fillLevel = ((Max_DISTANCE - distance) * 100) / Max_DISTANCE;
  }

  if (fillLevel < 0) fillLevel = 0;
  if (fillLevel > 100) fillLevel = 100;

  Serial.print("Fill Level: ");
  Serial.print(fillLevel);
  Serial.println("%");

  // Send data to ThingSpeak if Wi-Fi is connected
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = String(SERVER) + "?api_key=" + WRITE_APIKEY + "&field1=" + String(fillLevel);
    
    int retries = 0;
    int httpResponseCode = -1;
    while (retries < Max_RETRIES) {
      http.begin(url);
      httpResponseCode = http.GET();
      if (httpResponseCode > 0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        break; // Exit the retry loop if request is successful
      } else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
        retries++;
        Serial.print("Retrying... (");
        Serial.print(retries);
        Serial.println(")");
        delay(RETRY_DELAY); // Wait before retrying
      }
      http.end();
    }

    if (httpResponseCode <= 0) {
      Serial.println("Failed to send data after retries");
    }
  } else {
    Serial.println("WiFi Disconnected");
  }

  delay(15000); // Send data every 15 seconds
}
