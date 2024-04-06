![Github License](https://img.shields.io/github/license/dacarson/WeatherFlowApi) ![Github Release](https://img.shields.io/github/v/release/dacarson/WeatherFlowApi?display_name=tag)

# WeatherFlow-RainSensor

## Description
Arduino sketch that uses the [Tempest by WeatherFlow](https://weatherflow.com/tempest-weather-system/) broadcast weather data to control rain sensor on an automatic irrigation system.

Listen to Rain events/rain fall count being broadcast by WeatherFlow, and toggle the Rain sensor on my automatic irrigation system.

Used a [relay shield](https://www.wemos.cc/en/latest/d1_mini_shield/relay.html) on a [Wemos mini D1](https://www.wemos.cc/en/latest/d1/d1_mini.html), wiring the NC (Normally Closed) side to the Rain sensor on the irrigation system.

Used a [DC Power Shield](https://www.wemos.cc/en/latest/d1_mini_shield/dc_power.html) on the Wemos, the Wemos can be powered from the irrigation system with an [AC/DC converter](https://www.amazon.com/BeElion-Convertor-Current-Surveillance-Security/dp/B01JD6ASF8/) to convert it's 24VAC to DC and attaching to DC Power Shield (7-24V). Allowing the whole unit to fit inside the automatic irrigation system enclosure.

The amount of rain required to trigger sensor, and the amount of time before the rain sensor is disabled is controllable via a web page served by the sketch. 

![Configuration](https://github.com/dacarson/WeatherFlow-RainSensor/assets/44933987/6037dd06-60a9-4c86-8183-cc08467b8c6f)
![Status & Testing](https://github.com/dacarson/WeatherFlow-RainSensor/assets/44933987/2fb8c9b0-ac74-41dc-956f-6ce5004bc80c)

## Setup
Change the lines in `WeatherFlow-RainSensor.ino` to contain your WiFi SSID and Password:
```
const char *ssid     = "your_wifi_ssid";
const char *password = "your_wifi_password";
```
Make sure you upload the web files to a [LittleFS](https://github.com/earlephilhower/arduino-littlefs-upload) file system.

## License
This library is licensed under [MIT License](https://opensource.org/license/mit/)
