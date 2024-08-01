#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include "Conf.h"  // Configuration file for Wi-Fi and ThingSpeak credentials

//  pins for ultrasonic sensors
#define echoPin1 5   // Echo pin for sensor 1
#define trigPin1 18  // Trigger pin for sensor 1
#define echoPin2 19  // Echo pin for sensor 2
#define trigPin2 21  // Trigger pin for sensor 2

// Constants
const int Max_DISTANCE = 20;  // Maximum distance (height of the bin) in cm
const int Max_RETRIES = 3;    // Maximum number of retries for HTTP requests
const int RETRY_DELAY = 5000; // Delay between retries in milliseconds

// Variables to store sensor readings and calculations
long duration1, distance1;
long duration2, distance2;
int fillLevel1;
int fillLevel2;
bool sensor1Valid;
bool sensor2Valid;

void setup() {
  Serial.begin(9600);
  
  // Set pin modes for ultrasonic sensors
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);

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
  // Reset validity flags for sensors
  sensor1Valid = true;
  sensor2Valid = true;

  // Measure distance using sensor 1
  digitalWrite(trigPin1, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin1, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin1, LOW);

  duration1 = pulseIn(echoPin1, HIGH);
  distance1 = duration1 * 0.034 / 2;

  // Validate sensor 1 reading
  if (distance1 <= 0) {
    Serial.println("Error: Invalid distance reading from sensor 1");
    sensor1Valid = false;
  } else {
    Serial.print("Distance from sensor 1: ");
    Serial.print(distance1);
    Serial.println(" cm");

    // Calculate fill level for sensor 1
    fillLevel1 = (distance1 >= Max_DISTANCE) ? 100 : ((Max_DISTANCE - distance1) * 100) / Max_DISTANCE;
    fillLevel1 = constrain(fillLevel1, 0, 100);

    Serial.print("Fill Level from sensor 1: ");
    Serial.print(fillLevel1);
    Serial.println("%");
  }

  // Measure distance using sensor 2
  digitalWrite(trigPin2, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin2, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin2, LOW);

  duration2 = pulseIn(echoPin2, HIGH);
  distance2 = duration2 * 0.034 / 2;

  // Validate sensor 2 reading
  if (distance2 <= 0) {
    Serial.println("Error: Invalid distance reading from sensor 2");
    sensor2Valid = false;
  } else {
    Serial.print("Distance from sensor 2: ");
    Serial.print(distance2);
    Serial.println(" cm");

    // Calculate fill level for sensor 2
    fillLevel2 = (distance2 >= Max_DISTANCE) ? 100 : ((Max_DISTANCE - distance2) * 100) / Max_DISTANCE;
    fillLevel2 = constrain(fillLevel2, 0, 100);

    Serial.print("Fill Level from sensor 2: ");
    Serial.print(fillLevel2);
    Serial.println("%");
  }

  // Send data to server if WiFi is connected and at least one sensor is valid
  if (WiFi.status() == WL_CONNECTED && (sensor1Valid || sensor2Valid)) {
    HTTPClient http;
    String url = String(SERVER) + "?api_key=" + WRITE_APIKEY;

    // Append data for valid sensors to the URL
    if (sensor1Valid) {
      url += "&field1=" + String(fillLevel1);
    }
    if (sensor2Valid) {
      url += "&field2=" + String(fillLevel2);
    }

    // Attempt to send data with retries
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
    Serial.println("No valid sensor readings or WiFi Disconnected");
  }

  delay(15000); // Wait for 15 seconds before next measurement
}
