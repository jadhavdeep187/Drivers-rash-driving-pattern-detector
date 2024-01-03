#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <SoftwareSerial.h>
#include <TinyGPS++.h>
#include <WiFiClientSecure.h>

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>

Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(45);

#define ON_Board_LED 2  //--> Defining an On Board LED, used for indicators when the process of connecting to a wifi router

const char* ssid = "Abhishek"; //--> Your wifi name or SSID.
const char* password = "Abhi#2171"; //--> Your wifi password.

const int RXPin = 0, TXPin = 2;
const uint32_t GPSBaud = 9600;
SoftwareSerial gps_module(RXPin, TXPin);

TinyGPSPlus gps;

float gps_speed;
float no_of_satellites;
String satellite_orientation;

unsigned int move_index = 1;



//----------------------------------------Host & httpsPort
const char* host = "script.google.com";
const int httpsPort = 443;
//----------------------------------------

WiFiClientSecure client; //--> Create a WiFiClientSecure object.

String GAS_ID = "AKfycbwsnGny8EgwkCIQ-7p5eQ7-EdKFwdWrh3r109AVcX-FSCevWwZ9Unho0UyhihwtB8eQ"; //--> spreadsheet script ID

void setup(void) {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(500);
  
  gps_module.begin(GPSBaud);
  checkGPS();
  
  if(!accel.begin()) {
    Serial.println("Could not find a valid ADXL345 sensor, check wiring!");
    while(1);
  }
  accel.setRange(ADXL345_RANGE_16_G);
  
  WiFi.begin(ssid, password); //--> Connect to your WiFi router
  Serial.println("");
    
  pinMode(ON_Board_LED,OUTPUT); //--> On Board LED port Direction output
  digitalWrite(ON_Board_LED, HIGH); //--> Turn off Led On Board

  //----------------------------------------Wait for connection
  Serial.print("Connecting");
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
  int age = 21; 
}

void displayInfo() {
  if (gps.location.isValid()) {
    
  float latitude = (gps.location.lat());
  float longitude = (gps.location.lng());
  
  sensors_event_t event;
  accel.getEvent(&event);
  
  float x = event.acceleration.x;
  float y = event.acceleration.y;
  float z = event.acceleration.z;
  
    Serial.print("X: "); Serial.print(x); Serial.print("  ");
    Serial.print("Y: "); Serial.print(y); Serial.print("  ");
    Serial.print("Z: "); Serial.print(z); Serial.println();
  
    Serial.print("LAT:  ");
    Serial.println(latitude, 8);
    Serial.print("LONG: ");
    Serial.println(longitude, 8);
    gps_speed = gps.speed.kmph();
    Serial.print("Speed: ");
    Serial.println(gps_speed);
    no_of_satellites = gps.satellites.value();
    Serial.print("no_of_satellites: ");
    Serial.println(no_of_satellites);
    satellite_orientation = TinyGPSPlus::cardinal(gps.course.value());
    Serial.print("satellite_orientation");
    Serial.println(satellite_orientation);
    String string_lat =  String(latitude,8);
    String string_long =  String(longitude,8);
    String string_speed = String(gps_speed);
    int permi=1; // this veriable need to be change by using esp8266 webserver
    if(permi==1)
    {
    sendData(x,y,z,21, string_lat, string_long, "Male", "MotorCycle", "DeepJadhav", string_speed );
    }
  }
  Serial.println();
}

// Subroutine for sending data to Google Sheets
void sendData(float x, float y , float z, int age, String string_lat, String string_long, String gender, String bike_type, String driver_name, String string_speed ) {
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
  String string_x =  String(x);
  String string_y =  String(y); 
  String string_z =  String(z);
  String string_age =  String(age);
  
  String url = "/macros/s/" + GAS_ID + "/exec?x=" + string_x + "&y=" + string_y + "&z=" + string_z + "&a=" + string_age + "&lat=" + string_lat + "&long=" + string_long + "&gender=" + gender + "&bike_type=" + bike_type + "&driver_name=" +  driver_name + "&bike_speed=" + string_speed ;
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
