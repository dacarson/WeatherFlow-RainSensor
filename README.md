# WeatherFlow-RainSensor
Arduino sketch that uses WeatherFlow broadcast rain data to control rain sensor on an automatic irrigation system.

Listen to Rain events/rain fall count being broadcast by WeatherFlow, and toggle the Rain sensor on my automatic irrigation system.

Used a [relay shield](https://www.wemos.cc/en/latest/d1_mini_shield/relay.html) on a [Wemos mini D1](https://www.wemos.cc/en/latest/d1/d1_mini.html), wiring the NC (Normally Closed) side to the Rain sensor on the irrigation system.

Used a [DC Power Shield](https://www.wemos.cc/en/latest/d1_mini_shield/dc_power.html) on the Wemos, the Wemos can be powered from the irrigation system with an [AC/DC converter](https://www.amazon.com/BeElion-Convertor-Current-Surveillance-Security/dp/B01JD6ASF8/) to convert it's 24VAC to DC and attaching to DC Power Shield (7-24V). Allowing the whole unit to fit inside the automatic irrigation system enclosure.

The amount of rain required to trigger sensor, and the amount of time before the rain sensor is disabled is controllable via a web page served by the sketch. 
