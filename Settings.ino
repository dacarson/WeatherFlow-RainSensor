/*
Copyright (c) 2023 David Carson (dacarson)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <ESP_EEPROM.h>

// EEPROM stored data
const int RainSettings::EEPROMStorageSize = 64;
// Settings
const int RainSettings::RainFallAmountOffset = 0;
const int RainSettings::DryingTimeOffset = (RainFallAmountOffset + sizeof(RainFallAmountOffset));

RainSettings::RainSettings() {
  // Default Settings
  _rainFallAmount = 12; //(5 - 20mm)
  _dryingTime = 3; //(1 - 23 hrs)

  // Check to see if there are any in EEPROM
  EEPROM.begin(EEPROMStorageSize);
  loadFromEEPROM();
}

RainSettings::~RainSettings() {
  EEPROM.end();
}

// See if there are settings stored in the EEPROM
// and if so, restore the values
void RainSettings::loadFromEEPROM() {
  int tempRainFallAmount;
  EEPROM.get(RainFallAmountOffset, tempRainFallAmount);
  if (tempRainFallAmount > 0) {
    _rainFallAmount = tempRainFallAmount;
    EEPROM.get(DryingTimeOffset, _dryingTime);
    Serial.print("Restored settings from EEPROM. Rain fall amount ");
    Serial.print(_rainFallAmount);
    Serial.print(" and drying time ");
    Serial.println(_dryingTime);
  } else {
    Serial.println("Using default Rain fall amount and drying time");
  }
}

/*
  Store settings so they exist over reboots
  Rely on ESP_EEPROM to handle only writing if values have changed
*/
void RainSettings::saveToEEPROM() {
  EEPROM.put(RainFallAmountOffset, _rainFallAmount);
  EEPROM.put(DryingTimeOffset, _dryingTime);
  if (EEPROM.commit())
    Serial.println("Updated settings in EEPROM");
}

/* 
  Amount of rainfall needed over 24hr period to trigger sensor
  Measurements in mm
*/
int RainSettings::rainFallAmount() {
  return _rainFallAmount;
}

/*
  The time after it has stopped raining before the sprinklers 
  can turn back on.
  Measurements in hrs
*/
int RainSettings::dryingTime() {
  return _dryingTime;
}

/* 
  Update the Rain Fall Amount to the new value
  return the new rain fall amount
*/
int RainSettings::setRainFallAmount(int amount) {
  Serial.println("Updating rain fall amount");
  if (amount > 4 && amount < 21) {
    _rainFallAmount = amount;
    saveToEEPROM();
  }
  return _rainFallAmount;
}

/* 
  Update the Drying Time to the new value
  return the new drying time
*/
int RainSettings::setDryingTime(int time) {
  Serial.println("Updating drying time");
  if (time > 0 && time < 24) {
    _dryingTime = time;
    saveToEEPROM();
  }
  return _dryingTime;
}
