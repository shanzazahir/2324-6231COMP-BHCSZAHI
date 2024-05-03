#include "main.h"

void setup();

void loop() {

  int lightLevel = analogRead(lightSensorPin);
  // Check if it's night mode based on light level
  if (lightLevel > nightThreshold) {
       nightMode();
    
  } else {
      isNightMode = false;
      lastDayModeTime = millis();
      if (lastNightModeTime != 0) {
        // Calculate duration of previous night-time period
        unsigned long duration = millis() - lastNightModeTime;

        tft.print(duration);
      }
      char customKey = customKeypad.getKey();
  if (customKey != NO_KEY) {
    switch (customKey) {
      case '8':
      winterMode();
        break;

      case '2':
        germinatingMode();
        break;

      case '3':
       vegetativeMode();
        break;

      case '4':
        fruitingMode();
        break;

      case '5':
      displayMenu();
 char newKey = customKeypad.getKey();
if (newKey != NO_KEY) {
    if (newKey == '7') {
        tft.print("Edit Winter Mode");
        userEditMode('7');
    } else if (newKey == '6') {
        tft.print("Edit Germinating Mode");
        userEditMode('6');
    } else if (newKey == '9') {
        tft.print("Edit Vegetative Mode");
        userEditMode('9');
    } else if (newKey == '0') {
        tft.print("Edit Fruiting Mode");
        userEditMode('0');
    }
}

  }

    
  }
}

}
