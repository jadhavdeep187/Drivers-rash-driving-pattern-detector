
#include <SoftwareSerial.h>
#include <TinyGPS++.h>
#include <ESP8266WiFi.h>

#define ON_Board_LED 2
const int RXPin = 0, TXPin = 2;
const uint32_t GPSBaud = 9600;
SoftwareSerial gps_module(RXPin, TXPin);

TinyGPSPlus gps;

float gps_speed;
float no_of_satellites;
String satellite_orientation;

unsigned int move_index = 1;

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.print("HEllo World");
  gps_module.begin(GPSBaud);
  checkGPS();
  }


void checkGPS() {
  if (gps.charsProcessed() < 10) {
    Serial.println(F("No GPS detected: check wiring."));
  }
}

void loop() {
  
  while (gps_module.available() > 0) {
    if (gps.encode(gps_module.read()))
      displayInfo();
  }
}

void displayInfo() {
  if (gps.location.isValid()) {
    float latitude = (gps.location.lat());
    float longitude = (gps.location.lng());

    Serial.print("LAT:  ");
    Serial.println(latitude, 2);
    Serial.print("LONG: ");
    Serial.println(longitude, 2);
    gps_speed = gps.speed.kmph();
    Serial.print("Speed: ");
    Serial.println(gps_speed);
    no_of_satellites = gps.satellites.value();
    Serial.print("no_of_satellites: ");
    Serial.println(no_of_satellites);
    satellite_orientation = TinyGPSPlus::cardinal(gps.course.value());
    Serial.print("satellite_orientation");
    Serial.println(satellite_orientation);
  Serial.println();
  }
}
