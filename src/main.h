//#include <WiFi.h>
//#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Keypad.h>
#include "DHTesp.h"

const int DHT_PIN = 13;
const int redLed = 21;
const int greenLed = 5;
const int lightSensorPin = 36; 
const int nightThreshold = 2300;

bool isNightMode = false;
unsigned long lastDayModeTime = 0;
unsigned long lastNightModeTime = 0;

DHTesp dhtSensor;


const byte ROWS = 4; 
const byte COLS = 4; 

char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {34, 35, 32, 33}; 
byte colPins[COLS] = {25, 26, 27, 14}; 
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 

// const char* ssid = "BICLHR";
// const char* password = "BICLHR786";

#define TFT_DC 2
#define TFT_CS 15
#define TFT_MOSI 23
#define TFT_CLK 18
#define TFT_RST 4
#define TFT_MISO 19

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

const int MAX_READINGS = 1440; // Maximum number of readings to store (24 hours at 1 minute interval)
float tempReadings[MAX_READINGS];
float humReadings[MAX_READINGS];
unsigned long timeStamps[MAX_READINGS];
int numReadings = 0; // Number of readings stored

void displayMenu() {
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextSize(2);
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9341_WHITE);
  tft.println("Select Mode to Edit:");
  tft.println("1. Winter");
  tft.println("2. Germinating");
  tft.println("3. Vegetative");
  tft.println("4. Fruiting");
} 

void Start() {
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9341_WHITE);
  tft.println("\nSelect Mode");

  tft.setTextColor(ILI9341_GREEN);
  tft.println("1. Winter");
  tft.println("2. Germinating");
  tft.println("3. Vegetative");
  tft.println("4. Fruiting");
}

void setup() {
  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);

digitalWrite(redLed, HIGH);  
  digitalWrite(greenLed, HIGH);  
  // WiFi.begin(ssid, password, 6);
  tft.begin();
  tft.setRotation(1);

   tft.setTextColor(ILI9341_BLACK);
   tft.setTextSize(2);
  // tft.print("Connecting to WiFi");

  // while (WiFi.status() != WL_CONNECTED) {
  //   delay(100);
  //   tft.print(".");
  // }
  // tft.print("\nOK! IP=");
  // tft.println(WiFi.localIP());
  // delay(500);

  dhtSensor.setup(DHT_PIN, DHTesp::DHT11);
  delay(100);
  Start();
  
  // Initialize arrays
  for (int i = 0; i < MAX_READINGS; i++) {
    tempReadings[i] = 0.0;
    humReadings[i] = 0.0;
    timeStamps[i] = 0;
  }
}

void logData(float temperature, float humidity) {
  // This piece of code makes sure that the readings are logged until there are 1440 readings
  if (numReadings < MAX_READINGS) {
    tempReadings[numReadings] = temperature;
    humReadings[numReadings] = humidity;
    timeStamps[numReadings] = millis();
    numReadings++;
  } else { 
    // If maximum readings have reached, discard oldest reading and shift array
    for (int i = 0; i < MAX_READINGS - 1; i++) {
      tempReadings[i] = tempReadings[i + 1];
      humReadings[i] = humReadings[i + 1];
      timeStamps[i] = timeStamps[i + 1];
    }
    tempReadings[MAX_READINGS - 1] = temperature;
    humReadings[MAX_READINGS - 1] = humidity;
    timeStamps[MAX_READINGS - 1] = millis();
  }
}

void displaySensorData(float temperature, float humidity) {
  // This function will be called inside all the day modes along with the night mode
  // Display current sensor values
  tft.setCursor(0, 20);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.print("Current Temperature: ");
  tft.print(temperature);
  tft.println(" °C");
  tft.print("Current Humidity: ");
  tft.print(humidity);
  tft.println("%");

   static unsigned long lastLogTime = 0;
  if (millis() - lastLogTime >= 60000) {
    logData(temperature, humidity);
    lastLogTime = millis();
  }
  // Display minimum and maximum temperature/humidity over the last 24 hours
  float minTemp = tempReadings[0];
  float maxTemp = tempReadings[0];
  float minHum = humReadings[0];
  float maxHum = humReadings[0];
  for (int i = 1; i < numReadings; i++) {
    if (tempReadings[i] < minTemp) {
      minTemp = tempReadings[i];
    }
    if (tempReadings[i] > maxTemp) {
      maxTemp = tempReadings[i];
    }
    if (humReadings[i] < minHum) {
      minHum = humReadings[i];
    }
    if (humReadings[i] > maxHum) {
      maxHum = humReadings[i];
    }
  }
  tft.print("Min Temp (24h): ");
  tft.print(minTemp);
  tft.println(" °C");
  tft.print("Max Temp (24h): ");
  tft.print(maxTemp);
  tft.println(" °C");
  tft.print("Min Humidity (24h): ");
  tft.print(minHum);
  tft.println("%");
  tft.print("Max Humidity (24h): ");
  tft.print(maxHum);
  tft.println("%");
}
void nightMode() {

       tft.setCursor(0, 0);
       tft.fillScreen(ILI9341_BLACK);
       tft.println("Night Mode is activated");

      float temperature = dhtSensor.getTemperature();
      float humidity = dhtSensor.getHumidity();

       displaySensorData(temperature, humidity);
       
        if (temperature >= 20 && temperature <= 25 && humidity >= 60 && humidity <= 70){

      digitalWrite(redLed, HIGH);
      digitalWrite(greenLed, LOW);

      tft.print("Temperature and Humidity are within the desired range ");

        } else {

          digitalWrite(redLed, LOW);
          digitalWrite(greenLed, HIGH);

      tft.print("Temperature and Humidity are outside the desired range ");

        }
        
      isNightMode = true;
      lastNightModeTime = millis();
      if (lastDayModeTime != 0) {
        // Calculate duration of previous daytime period
        unsigned long duration = millis() - lastDayModeTime;

        tft.print(duration);
    }
    
    
}
void winterMode() {
       tft.setCursor(0, 0);
       tft.fillScreen(ILI9341_BLACK);
       tft.println("Winter Mode Selected");

       float temperature = dhtSensor.getTemperature();
       float humidity = dhtSensor.getHumidity();

       displaySensorData(temperature, humidity);

        if (temperature < 0) {

      digitalWrite(redLed, HIGH);
      digitalWrite(greenLed, LOW);

      tft.print("Temperature and Humidity are within the desired range ");

        } else {

          digitalWrite(redLed, LOW);
          digitalWrite(greenLed, HIGH);

      tft.print("Temperature and Humidity are outside the desired range ");

        }

      tft.println("2- Germinating Mode");
      tft.println("3- Vegetative Mode");
      tft.println("4- Fruiting Mode");

}

void germinatingMode() { 

       tft.setCursor(0, 0);
       tft.fillScreen(ILI9341_BLACK);
       tft.println("Germinating Mode Selected");

       float temperature = dhtSensor.getTemperature();
       float humidity = dhtSensor.getHumidity();

       displaySensorData(temperature, humidity);

        if (temperature >= 18 && temperature <= 32 && humidity >= 70 && humidity <= 95) {

      digitalWrite(redLed, HIGH);
      digitalWrite(greenLed, LOW);

      tft.print("Temperature and Humidity are within the desired range ");

        } else {

          digitalWrite(redLed, LOW);
          digitalWrite(greenLed, HIGH);

      tft.print("Temperature and Humidity are outside the desired range ");
      
        }
               
      tft.println("1- Winter Mode");
      tft.println("3- Vegetative Mode");
      tft.println("4- Fruiting Mode");


}

void vegetativeMode() {
  tft.setCursor(0, 0);
       tft.fillScreen(ILI9341_BLACK);
       tft.println("Vegetative Mode Selected");

       float temperature = dhtSensor.getTemperature();
       float humidity = dhtSensor.getHumidity();

       displaySensorData(temperature, humidity);

        if (temperature >= 20 && temperature <= 25 && humidity >= 60 && humidity <= 70) {

      digitalWrite(redLed, HIGH);
      digitalWrite(greenLed, LOW);

      tft.print("Temperature and Humidity are within the desired range ");

        } else {

          digitalWrite(redLed, LOW);
          digitalWrite(greenLed, HIGH);

      tft.print("Temperature and Humidity are outside the desired range ");
      
        }
      tft.println("1- Winter Mode");
      tft.println("2- Germinating Mode");
      tft.println("4- Fruiting Mode");
}

void fruitingMode() {
 tft.setCursor(0, 0);
       tft.fillScreen(ILI9341_BLACK);
       tft.println("Fruiting Mode Selected");

       float temperature = dhtSensor.getTemperature();
       float humidity = dhtSensor.getHumidity();

       displaySensorData(temperature, humidity);
        if (temperature <= 28 && humidity >= 40 && humidity <= 50) {

      digitalWrite(redLed, HIGH);
      digitalWrite(greenLed, LOW);

      tft.print("Temperature and Humidity are within the desired range ");

        } else {

          digitalWrite(redLed, LOW);
          digitalWrite(greenLed, HIGH);

      tft.print("Temperature and Humidity are outside the desired range ");
      
        }
      tft.println("1- Winter Mode");
      tft.println("2- Germinating Mode");
      tft.println("3- Vegetative Mode");
       
}

 float getUserTemp() {
  float temperature = 0.0;
  String input = "";

  while (true) {
   char key = customKeypad.getKey();
    if (key >= '0' && key <= '9') {
      input += key;
      tft.print(key);
    } else if (key == '*') {
      temperature = input.toFloat();
      if (temperature >= 0 && temperature <= 40) {
        break;
      } else {
        tft.println("Temperature must be between 0-40 C. Please re-enter.");
        input = "";
        delay(1000);
        tft.setCursor(0, tft.getCursorY());
        tft.println("Enter temperature (0-40 C):");
      }
    }
  }
  return temperature;
}
float getUserHumidity() {
  float humidity = 0.0;
  String input = "";

  while (true) {
   char key = customKeypad.getKey();
    if (key >= '0' && key <= '9') {
      input += key;
      tft.print(key);
    } else if (key == '*') {
      humidity = input.toFloat();
      if (humidity >= 0 && humidity <= 100) {
        break;
      } else {
        tft.println("Humidity must be between 0-100%. Please re-enter.");
        input = "";
        delay(1000);
        tft.fillScreen(ILI9341_BLACK);
        tft.setCursor(0, 4);
        tft.println("Enter humidity (0-100%):");
      }
    }
  }
  return humidity;
}
void userEditMode(char modeKey) {
  String mode = "Editable Mode";
  float minTemp = 0.0;
  float maxTemp = 0.0;
  float minHumidity = 0.0;
  float maxHumidity = 0.0;

  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0, 0);
  tft.println(mode);
  
  tft.println("Enter minimum temperature (0-40 C):");
  minTemp = getUserTemp();
  
  tft.println("Enter maximum temperature (0-40 C):");
  maxTemp = getUserTemp();
  
  while (minTemp >= maxTemp) {
    tft.println("Minimum temperature must be less than maximum temperature. Please re-enter.");
    tft.fillScreen(ILI9341_BLACK);
    tft.setCursor(0, 0);
    tft.println(mode);
    tft.println("Minimum temperature must be less than maximum temperature. Please re-enter.");
    minTemp = getUserTemp();
    tft.println("Enter maximum temperature (0-40 C):");
    maxTemp = getUserTemp();
  }

  tft.println("Enter minimum humidity (0-100%):");
  minHumidity = getUserHumidity();

  tft.println("Enter maximum humidity (0-100%):");
  maxHumidity = getUserHumidity();
  
  while (minHumidity >= maxHumidity) {
    tft.println("Minimum humidity must be less than maximum humidity. Please re-enter.");
    tft.fillScreen(ILI9341_BLACK);
    tft.setCursor(0, 0);
    tft.println(mode);
    tft.println("Minimum humidity must be less than maximum humidity. Please re-enter.");
    minHumidity = getUserHumidity();
    tft.println("Enter maximum humidity (0-100%):");
    maxHumidity = getUserHumidity();
  }
  
  // Display the entered values
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0, 0);
  tft.println(mode);
  tft.println();
  tft.print("Entered Minimum Temperature: ");
  tft.print(minTemp);
  tft.println(" C");
  tft.print("Entered Maximum Temperature: ");
  tft.print(maxTemp);
  tft.println(" C");
  tft.print("Entered Minimum Humidity: ");
  tft.print(minHumidity);
  tft.println("%");
  tft.print("Entered Maximum Humidity: ");
  tft.print(maxHumidity);
  tft.println("%");

      float temp = dhtSensor.getTemperature();
      float hum = dhtSensor.getHumidity();
   if (temp < minTemp || temp > maxTemp || hum < minHumidity || hum > maxHumidity) {

      digitalWrite(redLed, HIGH);
      digitalWrite(greenLed, LOW); 

      tft.print("Temperature and Humidity are within the desired range ");

    } else {
          digitalWrite(redLed, LOW);
          digitalWrite(greenLed, HIGH);

      tft.print("Temperature and Humidity are outside the desired range ");
    }
  
}
