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

#if defined (ESP8266)
  #include <ESP8266WebServer.h>
  extern ESP8266WebServer server;
#else if defined (ESP32_DEV)
  #include <WebServer.h>
  extern WebServer server;
#endif

// Default Settings defined in WeatherFlow-RainSensor.ino
// Keep track of when the Rain started
extern long rainStartEpoch;

// Need a stack of the last 24 hrs
// that has the rainfall total.
extern float rainInHour[];
extern int currentHour; // so we can see if we have moved into a new hour

// Rain Settings
extern RainSettings settings;

void handleRoot() {
  char temp[100];

  // Handle any arguements passed in
  if (server.args() > 1) {
    String rainFallAmountStr = server.arg("rainFallAmount");
    settings.setRainFallAmount(rainFallAmountStr.toInt());

    String dryingTimeStr = server.arg("dryingTime");
    settings.setDryingTime(dryingTimeStr.toInt());
  }

  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send ( 200, "text/html", "" );

// Send HTML header
  server.sendContent(F("<html>\
  <head>\
    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\
    <title>WeatherFlow Rain Sensor</title>\
    <link rel=\"stylesheet\" href=\"main.css\">\
  </head>\
"));

// Send current configuation
server.sendContent(F("<body>\
<h1>IoT Rain Sensor</h1>\
  <div class='data-box'>\
  <div id='configuration' class='center-data' style='display: block;'>\
  <h2>Configuration</h2>\
"));
server.sendContent(F("<p><p><form><B><label for=\"rainFallAmount\">24hr Rainfall Threshold (mm):</label>"));
server.sendContent(F("<select name=rainFallAmount>"));
for (int i = 5; i < 21; i++) {
  if (i == settings.rainFallAmount())
    server.sendContent(F("<option selected>"));
  else
    server.sendContent(F("<option>"));
  sprintf(temp, "%d", i);
  server.sendContent(temp);
  server.sendContent(F("</option>"));
}
server.sendContent(F("</select>"));
server.sendContent(F("<p> <label for=\"dryingTime\">Delay Before Turning Back On (hours):</label>"));
server.sendContent(F("<select name=dryingTime>"));
for (int i = 1; i < 24; i++) {
  if (i == settings.dryingTime())
    server.sendContent(F("<option selected>"));
  else
    server.sendContent(F("<option>"));
  sprintf(temp, "%d", i);
  server.sendContent(temp);
  server.sendContent(F("</option>"));
}
server.sendContent(F("</select>"));
server.sendContent(F("</b>"));
server.sendContent(F("<button type='submit' value=\"Update\" class=\"bottom-right-button\">Save Settings</button></form>"));

server.sendContent(F("</div><div id=\"status\" class=\"center-data\" style=\"display: none;\">"));
// Send current status
server.sendContent(F("<h2>Status</h2>Rain Sensor: "));
if (relayOffTime == -1) {
  server.sendContent(F("Not "));
}
server.sendContent(F("Triggered<br>"));
if (relayOffTime > 0) {
  server.sendContent(F("Rain sensor will reset at: <span id='relayOffTime'></span><br>"));
  server.sendContent(F("<script> var date = new Date("));
  sprintf(temp, "%ld", relayOffTime);
  server.sendContent(temp);
  server.sendContent(F(" * 1000); var time = date.toLocaleTimeString();"));
  server.sendContent(F("document.getElementById('relayOffTime').innerHTML = time;</script>"));
}
server.sendContent(F("Rain detected: <span id='rainStartTime'></span><br>"));
server.sendContent(F("<script> var date = new Date("));
sprintf(temp, "%ld", rainStartEpoch);
server.sendContent(temp);
server.sendContent(F(" * 1000); var time = date.toLocaleTimeString(); var d = date.toDateString();"));
server.sendContent(F("document.getElementById('rainStartTime').innerHTML = d + ' ' + time;</script>"));
server.sendContent(F("Rainfall: Last hour: "));
sprintf(temp, "%.2f", rainInHour[currentHour]);
server.sendContent(temp);
server.sendContent(F("mm, Last 24hrs: "));
float total = 0;
for (int hour = 0; hour < 24; hour++)
    total += rainInHour[hour];
sprintf(temp, "%.2f", total);
server.sendContent(temp);  
server.sendContent(F("mm<p><hr>"));

// Testing
// Send current status
server.sendContent(F("<h2>Testing</h2>Send Rain start event <button type=\"button\" onclick=\"rainstart()\">Now</button><br>"));
server.sendContent(F("<script>\
function rainstart()\
{\
    var xhr = new XMLHttpRequest();\
    xhr.open(\"GET\", \"/rainstart\", true); \
    xhr.send(null);\
}\
</script>"));

server.sendContent(F("Send observation event with rain total 3.8 <button type=\"button\" onclick=\"obstest()\">Now</button><br>"));
server.sendContent(F("<script>\
function obstest()\
{\
    var xhr = new XMLHttpRequest();\
    xhr.open(\"GET\", \"/obstest\", true); \
    xhr.send(null);\
}\
</script>"));

server.sendContent(F("Reset all stored rain data <button type=\"button\" onclick=\"reset()\">Reset</button><br>"));
server.sendContent(F("<script>\
function reset()\
{\
    var xhr = new XMLHttpRequest();\
    xhr.open(\"GET\", \"/reset\", true); \
    xhr.send(null);\
}\
</script>"));

// Send build info
  server.sendContent(F("<p><pre>Compiled: "));
  server.sendContent(F(__DATE__));
  server.sendContent( F(", "));
  server.sendContent(F(__TIME__));
  server.sendContent( F(", GCC: "));
  server.sendContent(F(__VERSION__));
  server.sendContent(F("</pre></p>\
  </div></div>\
  <div class=\"data-box\">\
    <div class=\"link-box\">\
      <a href=\"#\" id=\"swapLink\">Status</a>\
    </div>\
  </div>\
  <script src=\"main.js\"></script>\
  </body></html>"));
  server.client().stop();

}