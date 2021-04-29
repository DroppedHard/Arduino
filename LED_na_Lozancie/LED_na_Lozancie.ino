#include "EEPROM.h"
#include <dummy.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
//#include <WiFi101.h>
#include <Losant.h>

// WiFi credentials.
const char* WIFI_SSID = "XXX";
const char* WIFI_PASS = "XXX";

// Losant credentials.
const char* LOSANT_DEVICE_ID = "XXX";
const char* LOSANT_ACCESS_KEY = "XXX";
const char* LOSANT_ACCESS_SECRET = "XXX";
const int BUTTON_PIN = D4;
const int LED_PIN = LED_BUILTIN;
int buttonState = 0;
bool ledState = false;

const int greenLEDPin = D0;    // LED connected to digital pin
const int redLEDPin = D1;     // LED connected to digital pin
const int blueLEDPin = D2;    // LED connected to digital pin

int redValue = 0; // value to write to the red LED
int greenValue = 0; // value to write to the green LED
int blueValue = 0; // value to write to the blue LED

WiFiClientSecure wifiClient;

#define EEPROM_SIZE 64
int freq = 22000; // in Hz,
int resolution = 10;

// For an unsecure connection to Losant.
// WiFiClient wifiClient;

LosantDevice device(LOSANT_DEVICE_ID);



void setup() {
  Serial.begin(115200);
  EEPROM.begin(512);
  while(!Serial) { }
  pinMode(BUTTON_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(greenLEDPin, OUTPUT);
  pinMode(redLEDPin, OUTPUT);
  pinMode(blueLEDPin, OUTPUT);
  int redValue = EEPROM.read(509);
  int greenValue = EEPROM.read(510);
  int blueValue = EEPROM.read(511);
  Serial.println(redValue);
  Serial.println(greenValue);
  Serial.println(blueValue);
  /*ledcSetup(1, freq, resolution);
  ledcAttachPin(greenLEDPin, 1);
  ledcSetup(2, freq, resolution);
  ledcAttachPin(redLEDPin, 2);
  ledcSetup(3, freq, resolution);
  ledcAttachPin(blueLEDPin, 3);*/  
 /* if (!EEPROM.begin(EEPROM_SIZE))
  {
    Serial.println("failed to initialise EEPROM"); delay(1000000);
  }*/

  // Register the command handler to be called when a command is received
  // from the Losant platform.
  device.onCommand(&handleCommand);

  connect();
}

void loop() {

  bool toReconnect = false;

  if(WiFi.status() != WL_CONNECTED) {
    Serial.println("Disconnected from WiFi");
    toReconnect = true;
  }

  if(!device.connected()) {
    Serial.println("Disconnected from Losant");
    Serial.println(device.mqttClient.state());
    toReconnect = true;
  }

  if(toReconnect) {
    connect();
  }

  device.loop();

  int currentRead = digitalRead(BUTTON_PIN);

  if(currentRead != buttonState) {
    buttonState = currentRead;
    if(buttonState) {
      buttonPressed();
    }
  }
  analogWrite(redLEDPin, redValue);
  analogWrite(greenLEDPin, greenValue);
  analogWrite(blueLEDPin, blueValue);

  delay(100);
}

void buttonPressed() {
  Serial.println("Button Pressed!");

  // Losant uses a JSON protocol. Construct the simple state object.
  // { "button" : true }
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["button"] = true;

  // Send the state to Losant.
  device.sendState(root);

  EEPROM.write(509,redValue);
  EEPROM.write(510,greenValue);
  EEPROM.write(511,blueValue);
  EEPROM.commit();
  Serial.println(redValue);
  Serial.println(greenValue);
  Serial.println(blueValue);
  int cz = EEPROM.read(509);
  int z = EEPROM.read(510);
  int n = EEPROM.read(511);
  Serial.println(cz);
  Serial.println(z);
  Serial.println(n);
}

void toggle() {
  Serial.println("Toggling LED.");
  ledState = !ledState;
  digitalWrite(LED_PIN, ledState ? HIGH : LOW);
}
void kolor() {
  Serial.println("bułka");
  //JsonObject& payload = *command->payload;
  //redValue = payload["red"];
}

// Called whenever the device receives a command from the Losant platform.
void handleCommand(LosantCommand *command) {
  Serial.print("Command received: ");
  Serial.println(command->name);

  JsonObject& payload = *command->payload;
  
  if(strcmp(command->name, "toggle") == 0) {
    toggle();
  }
  if (strcmp(command->name, "ustaw kolor") == 0) {
    kolor();
     JsonObject& payload = *command->payload;
    redValue = payload["RED"];
    greenValue = payload["GREEN"];
    blueValue = payload["BLUE"];
    Serial.print(redValue);
    Serial.print(greenValue);
    Serial.print(blueValue);
    
    
  }
}

void connect() {

  // Connect to Wifi.
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Connect to Losant.
  Serial.println();
  Serial.print("Connecting to Losant...");

  device.connectSecure(wifiClient, LOSANT_ACCESS_KEY, LOSANT_ACCESS_SECRET);

  // For an unsecure connection.
  // device.connect(wifiClient, ACCESS_KEY, ACCESS_SECRET);

  while(!device.connected()) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("Connected!");
}
