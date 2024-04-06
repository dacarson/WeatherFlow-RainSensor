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

/*
WeatherFlow-RainSensor

Listen to Rain events/rain fall count, and toggle the Rain sensor on my
automatic irrigation system.

Using a relay shield on a Wemos mini D1, wiring the NC (Normally Closed) 
side to the Rain sensor on the irrigation system. 

Using a DC Power Shield on the Wemos, the Wemos can be powered from the 
irrigation system with an AC/DC to convert it's 24VAC to DC and attaching
to DC Power Shield (7-24V)
*/

#if defined (ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266WebServer.h>
  #include <ESP8266mDNS.h>
#else if defined (ESP32_DEV)
  #include <WiFi.h>
  #include <WebServer.h>
  #include <ESPmDNS.h>
#endif

#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <WeatherFlowUdp.h>
#include "Settings.h"

// Pins
const int led = LED_BUILTIN;
#if defined (ESP8266)
  const int relayPin = D1;
#else if defined (ESP32_DEV)
  const int relayPin = 22;
#endif

// WiFi Credentials
const char *ssid     = "your_wifi_name";
const char *password = "your_wifi_password";

// Construct a global settings object
RainSettings settings;

// We'll use time provided by Network Time Protocol to get EpochTime 
// and use that with WeatherFlowData as that uses EpochTime too.
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Construct a webserver
#if defined (ESP8266)
  ESP8266WebServer server(80);
#else if defined (ESP32_DEV)
  WebServer server(80);
#endif
// defined in RootWebPage
void handleRoot();


// Construct a weather object
WeatherFlowUdp currentWeather;

// Keep track of when the Rain started
long rainStartEpoch = 0;

// Need a stack of the last 24 hrs
// that has the rainfall total.
float rainInHour[24];
int currentHour; // so we can see if we have moved into a new hour

// When it increases above rainFallAmount, turn on sensor.
// When it drops below rainFallAmount, 
// for longer than dryingTime, turn off sensor
long relayOffTime = -1; //EpochTime


// Callback for when new WeatherFlow data arrives
void handleNewObject(WeatherFlowData::Object obj, void* context) {
  if (obj == WeatherFlowData::RAIN) {
    rainStartEpoch = (long)currentWeather.getValue(WeatherFlowData::Time_Epoch);
  }
  if (obj == WeatherFlowData::TEMPEST||obj == WeatherFlowData::SKY) {
    int hour = timeClient.getHours();
    float rainLastMinute = (float)currentWeather.getValue(WeatherFlowData::Rain_Last_Minute);
    if (currentHour == hour)
      rainInHour[hour] += rainLastMinute;
    else
      rainInHour[hour] = rainLastMinute;

    // If we got some rain, but not triggered yet, 
    // double blink the LED
    if (rainLastMinute && relayOffTime < 0) {
        digitalWrite (led, LOW);
        delay(250);
        digitalWrite (led, HIGH);
        delay(250);
        digitalWrite (led, LOW);
        delay(250);
        digitalWrite (led, HIGH);
    }

    // If we got no rain, but we are triggered,
    // single slower LED blink
    if (!rainLastMinute && relayOffTime >= 0) {
        digitalWrite (led, LOW);
        delay(1000);
        digitalWrite (led, HIGH);
    }

    // Last time we saw that it was raining.
    if (rainLastMinute)
      rainStartEpoch = (long)currentWeather.getValue(WeatherFlowData::Time_Epoch);

    currentHour = hour;

    // Checked 24hr rain fall total
    float totalAccumulated = 0;
    for (int i = 0; i < 24; i++) {
      totalAccumulated += rainInHour[i];
    }
    if (totalAccumulated >= settings.rainFallAmount())
      enableRainSensor();

    if (relayOffTime > 0 && relayOffTime <= timeClient.getEpochTime())
      disableRainSensor();
  }
}

// OK to call this even if rain sensor is already tripped
void enableRainSensor() {
  Serial.println("Rain Detected: Yes");
  // Calculate the new rain sensing off time
  relayOffTime = timeClient.getEpochTime() + (60 * 60 * settings.dryingTime());
  digitalWrite(relayPin, HIGH); // turn on relay
  digitalWrite (led, LOW);
}

// OK to call this even if rain sensor is already tripped
void disableRainSensor() {
  Serial.println("Rain Detected: No");
  relayOffTime = -1;
  digitalWrite(relayPin, LOW); // turn on relay
  digitalWrite (led, HIGH);
}

void rainStartTest() {
  Serial.println("Testing Rain Start");
  const char *rainStartTest = 
  "{\"serial_number\": \"SK-00008453\", \"type\":\"evt_precip\", \"hub_sn\": \"HB-00000001\",\"evt\":[%ld]}";
  char *temp = (char*)malloc(strlen(rainStartTest) + 10);
  sprintf(temp, rainStartTest, timeClient.getEpochTime());
  currentWeather.processPacket(temp);
  free(temp);
  server.send ( 200, "text/plain", "OK" );
  server.client().stop();
}

void observationTest() {
  Serial.println("Testing Rain Observation event with 3.8mm of rain");
  const char *observationTest = 
  "{\"serial_number\": \"SK-00008453\",\"type\":\"obs_sky\",\"hub_sn\": \"HB-00000001\",\
	  \"obs\":[[%ld,9000,10,3.8,2.6,4.6,7.4,187,3.12,1,130,null,0,3]], \"firmware_revision\": 29}";
  char *temp = (char*)malloc(strlen(observationTest) + 10);
  sprintf(temp, observationTest, timeClient.getEpochTime());
  currentWeather.processPacket(temp);
  free(temp);
  server.send ( 200, "text/plain", "OK" );
  server.client().stop();
}

void resetRainData() {
  Serial.println("Reseting all rain data");
  for (int i = 0; i < 24; i++) 
    rainInHour[i] = 0;
  rainStartEpoch = 0;
  disableRainSensor();

  server.send ( 200, "text/plain", "OK" );
  server.client().stop();
}

void setup() {
  pinMode ( led, OUTPUT );
  pinMode ( relayPin, OUTPUT);
	Serial.begin ( 115200 );
	WiFi.begin ( ssid, password );
	Serial.println ( F("") );
  Serial.print ( F("Connecting to WiFi "));

	// Wait for connection
	while ( WiFi.status() != WL_CONNECTED ) {
		delay ( 500 );
		Serial.print ( F(".") );
	}
	Serial.println ( F("") );
	Serial.print (F("Connected to ") );
	Serial.println ( ssid );
	Serial.print ( F("IP address: ") );
	Serial.println ( WiFi.localIP() );

  // Start the timeclient
  timeClient.begin();
  // update every 7 days (1000ms * 60mins * 60hrs * 24hrs * 5days)
  timeClient.setUpdateInterval(1000 * 60 * 60 * 24 * 5);

  // Register the handler function
  currentWeather.registerCallback(handleNewObject);
  currentWeather.begin();

  // Reset state
  disableRainSensor();

  // Setup Web Server
	server.on ("/", handleRoot);
  server.on ("/rainstart", rainStartTest);
  server.on ("/obstest", observationTest);
  server.on ("/reset", resetRainData);
  server.begin();
	Serial.println ( "HTTP server started" );

}

void loop() {
  timeClient.update();
  currentWeather.update();
  server.handleClient();
}
