#include <ArduinoHttpClient.h>
#include <WiFiS3.h>
#include "settings.h"
#include <dht11.h>

#define DHT11PIN 8

int photoTransistorPin = A0;       
int blueLEDPin = 9;         
int photoTransistorValue = 0;    
int blueLedValue = 0;          
int darkThreshold = 60; //ändra för känslighet

//WiFi- och HTTP-inställningar
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASSWORD;
char serverAdress[] = "clownfish-app-2jcw3.ondigitalocean.app";
int port = 443;

//DHT11-sensor
dht11 DHT11;

WiFiSSLClient wifi;
HttpClient client = HttpClient(wifi, serverAdress, port);

void setup() {
  Serial.begin(9600);

  //Setup för WiFi
  Serial.println("Ansluter till wifi");
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println(".");
    delay(500);
  }
  Serial.println("Ansluten till WiFi");

  //Setup för fototransistor och LED
  pinMode(photoTransistorPin, INPUT);
  pinMode(blueLEDPin, OUTPUT);
}

void loop() {
  //Fototransistoravläsning och LED-kontroll
  photoTransistorValue = analogRead(photoTransistorPin); // Läs från fototransistorn
  delay(5);
  
  Serial.print("Raw Sensor value \t Blue: ");
  Serial.println(photoTransistorValue);

  if (photoTransistorValue < darkThreshold) {
    blueLedValue = photoTransistorValue / 4;  // Konvertera till 0-255
    analogWrite(blueLEDPin, blueLedValue);
    Serial.println("It's dark, turning on the LED.");
  } else {
    analogWrite(blueLEDPin, 0);
    Serial.println("It's bright, turning off the LED.");
  }

  //DHT11-avläsning och POST till server
  int chk = DHT11.read(DHT11PIN);
  String postData = "{\"humidity\":" + String((float)DHT11.humidity, 1) + 
                    ", \"celsius\":" + String((float)DHT11.temperature, 1) + 
                    ", \"photoTransistorValue\":" + String(photoTransistorValue) + "}";
  
  Serial.print("JSON som skickas: ");
  Serial.println(postData);

  Serial.println("Skickar POST");
  
  client.beginRequest();
  client.post("/post-dht11-sensor-data");

  client.sendHeader("Content-Type", "application/json");
  client.sendHeader("Content-Length", postData.length());
  
  client.beginBody();
  client.print(postData);
  client.endRequest();

  int statusCode = client.responseStatusCode();
  String response = client.responseBody();

  if (statusCode == 200) {
    Serial.print("Status code: ");
    Serial.println(statusCode);
    Serial.print("Response: ");
    Serial.println(response);
    Serial.println("Humidity (%): ");
    Serial.println((float)DHT11.humidity, 1);
    Serial.println("Temperature (C): ");
    Serial.println((float)DHT11.temperature, 1);
  } else if (statusCode == -3) {
    Serial.print("Status code: ");
    Serial.println(statusCode);
    Serial.print("Response: ");
    Serial.println(response);
  }

  delay(3000); //5min
}