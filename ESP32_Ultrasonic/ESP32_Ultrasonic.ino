#include <Wifi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <Conf.h>   // configuration file for wifi and thingspeak credentials

#define echoPin 2
#define trigPin 4

const int Max_DISTANCE = 100; //maximum distance(height of the bin) in cm

long duration, distance;
int fillLevel;


void setup() {
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  Wifi.begin(WIFI_SSID, WIFI_PASSWORD);
  while(Wifi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.Println("");
  Serial.println("Wifi connected");
  Serial.println("IP address: ");
  Serial.println(Wifi.localIP());

}

void loop() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW):

  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  //calculate bin as percentage in relation to distance
  fillLevel = 100 - ((distance * 100) / Max_DISTANCE);
  if (fillLevel < 0) fillLevel = 0;
  if (fillLevel > 100) fillLevel = 100
  Serial.print("Fill Level: ");
  Serial.print(fillLevel);
  Serial.println("%");

}
