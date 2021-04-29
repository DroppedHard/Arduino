//#include <SoftwareSerial.h>
#include <Losant.h>
#include <Wire.h>
#include "PMS.h"
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
PMS pms(Serial);
PMS::DATA data;
#define I2C_ADDR    0x3F // <<----- Add your address here.  Find it from I2C Scanner
#define BACKLIGHT_PIN     3
#define En_pin  2
#define Rw_pin  1
#define Rs_pin  0
#define D4_pin  4
#define D5_pin  5
#define D6_pin  6
#define D7_pin  7

// access key: 3480e774-36f7-4fa5-ac53-62be00958d9e
// Access secret bd53958c523fd5c67a03a9af2849d5dbd00b6f0f57858d963efeab3187c88953

// WiFi network info.
const char* WIFI_SSID = "XXX";
const char* WIFI_PASS = "XXX";

// Losant credentials.
const char* LOSANT_DEVICE_ID = "XXX";
const char* LOSANT_ACCESS_KEY = "XXX";
const char* LOSANT_ACCESS_SECRET = "XXX";

// Create a secure WiFi client. This can also be an instance of the unsecured
// WiFiClient class, but the secure TLS client is recommended.
WiFiClientSecure wifiClient;

// Create an instance of a Losant device.
LosantDevice device(LOSANT_DEVICE_ID);

#ifdef ESP32
#pragma message(THIS EXAMPLE IS FOR ESP8266 ONLY!)
#error Select ESP8266 board.
#endif

const int analogInPin = A0;  // Analog input pin that battery is being measured
int sensorValue = 0;        // value read from the A0
static char outstr[10];

void setup()
{
  Serial.begin(9600);
  Serial1.begin(9600);
//  Serial2.begin(9600);
  Serial.println();
  Serial.println("Status\tHumidity (%)\tTemperature (C)\t(F)\tHeatIndex (C)\t(F)");
  String thisBoard= ARDUINO_BOARD;
  Serial.println(thisBoard);
  connect();
}

void loop()
{
  if (pms.read(data))
  {
    //lcd.setCursor (0,1);
    //lcd.print("P1:");
    //lcd.print(data.PM_AE_UG_1_0);
    Serial1.print("PM 1.0 (ug/m3): ");
    Serial1.println(data.PM_AE_UG_1_0);
    //lcd.setCursor (5,1);
    //lcd.print("P2:");
    //lcd.print(data.PM_AE_UG_2_5);
    Serial1.print("PM 2.5 (ug/m3): ");
    Serial1.println(data.PM_AE_UG_2_5);
    lcd.setCursor (10,1);
    lcd.print("P3:");
    lcd.print(data.PM_AE_UG_10_0);
    //Serial1.print("PM 10.0 (ug/m3): ");
    //Serial1.println(data.PM_AE_UG_10_0);

    //Serial1.println();
  
  delay(dht.getMinimumSamplingPeriod()+500);
  sensorValue = analogRead(analogInPin);
  float voltage = (sensorValue / 140.0);
  float humidity = dht.getHumidity();
  float temperature = dht.getTemperature();
dtostrf(voltage,4,2,outstr);
  Serial.print(dht.getStatusString());
  Serial.print("\t");
  Serial.print(humidity, 1);
  lcd.setCursor (0,0);
  lcd.print("T:");
  lcd.print(temperature,DEC);
  //lcd.setCursor (0,1);
  //lcd.print("H:");
  //lcd.print(humidity,DEC);
  lcd.setCursor(4,0);
  lcd.print(" V:");
  lcd.print(voltage);
  Serial.print("\t\t");
  Serial.print(temperature, 1);
  Serial.print("\t\t");
  Serial.print(dht.toFahrenheit(temperature), 1);
  Serial.print("\t\t");
  Serial.print(dht.computeHeatIndex(temperature, humidity, false), 1);
  Serial.print("\t\t");
  Serial.println(dht.computeHeatIndex(dht.toFahrenheit(temperature), humidity, true), 1);
  Serial.print("V:");
  Serial.print(voltage);
  Serial.print("\t\t");
  Serial.print(0xe2);
  delay(200);
  
}
}

// Connects to WiFi and then to the Losant Platform.
void connect() {

  // Connect to WiFi.
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Connect to Losant.
  Serial.println();
  Serial.print("Connecting to Losant...");

  // Connect the device instance to Losant using TLS encryption.
  device.connectSecure(wifiClient, LOSANT_ACCESS_KEY, LOSANT_ACCESS_SECRET);

  while(!device.connected()) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("Connected!");
}
