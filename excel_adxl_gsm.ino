
#include <SoftwareSerial.h>
#include <TinyGPS++.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <WiFiClientSecure.h>

const int RXPin = 0, TXPin = 2;
const uint32_t GPSBaud = 9600;
SoftwareSerial gps_module(RXPin, TXPin);
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(1345);

TinyGPSPlus gps;

float gps_speed;
float no_of_satellites;
String satellite_orientation;

char ssid[] = "Abhishek";
char pass[] = "Abhi#2171";

unsigned int move_index = 1;

//----------------------------------------Host & httpsPort
const char* host = "script.google.com";
const int httpsPort = 443;
//----------------------------------------

WiFiClientSecure client; //--> Create a WiFiClientSecure object.

String GAS_ID = "AKfycbxqVk4hj5z853WIyFQJlTOdBWPBlE9bxUdxZK0reHXRfXkA7tOxNVogQa4gTQNwD3Gs"; //--> spreadsheet script ID
#define ON_Board_LED 2  //--> Defining an On Board LED, used for indicators when the process of connecting to a wifi router

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.print("HEllo World");
  gps_module.begin(GPSBaud);
  checkGPS();
  if(!accel.begin()) {
    Serial.println("Could not find a valid ADXL345 sensor, check wiring!");
    while(1);
  }
  accel.setRange(ADXL345_RANGE_16_G);
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    //----------------------------------------Make the On Board Flashing LED on the process of connecting to the wifi router.
    digitalWrite(ON_Board_LED, LOW);
    delay(250);
    digitalWrite(ON_Board_LED, HIGH);
    delay(250);
    //----------------------------------------
  }
  //----------------------------------------
  digitalWrite(ON_Board_LED, HIGH); //--> Turn off the On Board LED when it is connected to the wifi router.
  Serial.println("");
  Serial.print("Successfully connected to : ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  //----------------------------------------

  client.setInsecure();
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
   sensors_event_t event;
   accel.getEvent(&event);

  float x = event.acceleration.x;
  float y = event.acceleration.y;
  float z = event.acceleration.z;
  float latitude = (gps.location.lat());
  float longitude = (gps.location.lng());
  
  if (gps.location.isValid()) {


    Serial.print("X: "); Serial.print(x); Serial.print("  ");
    Serial.print("Y: "); Serial.print(y); Serial.print("  ");
    Serial.print("Z: "); Serial.print(z); Serial.println();
    Serial.print("LAT:  ");Serial.print(latitude, 6);
    Serial.print(" LONG: ");Serial.println(longitude, 6);
    gps_speed = gps.speed.kmph();
    Serial.print("Speed: ");
    Serial.println(gps_speed);

    
    //no_of_satellites = gps.satellites.value();
    //Serial.print("no_of_satellites: ");
    //Serial.println(no_of_satellites);
    //satellite_orientation = TinyGPSPlus::cardinal(gps.course.value());
    //Serial.print("satellite_orientation");
    //Serial.println(satellite_orientation);

  sendData(x, y); //--> Calls the sendData Subroutine
  delay(1000);
  }
  Serial.println();
}
// Subroutine for sending data to Google Sheets
void sendData(float x, float y) {
  Serial.println("==========");
  Serial.print("connecting to ");
  Serial.println(host);
  
  //----------------------------------------Connect to Google host
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }
  //----------------------------------------

  //----------------------------------------Processing data and sending data
  String string_x=  String(x);
  String string_y =  String(y);  
  String url = "/macros/s/" + GAS_ID + "x" + string_x + "y" + string_y;
  Serial.print("requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
         "Host: " + host + "\r\n" +
         "User-Agent: BuildFailureDetectorESP8266\r\n" +
         "Connection: close\r\n\r\n");

  Serial.println("request sent");
  //----------------------------------------

  //----------------------------------------Checking whether the data was sent successfully or not
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  String line = client.readStringUntil('\n');
  if (line.startsWith("{\"state\":\"success\"")) {
    Serial.println("esp8266/Arduino CI successfull!");
  } else {
    Serial.println("esp8266/Arduino CI has failed");
  }
  Serial.print("reply was : ");
  Serial.println(line);
  Serial.println("closing connection");
  Serial.println("==========");
  Serial.println();
  //----------------------------------------
} 
//==============================================================================
