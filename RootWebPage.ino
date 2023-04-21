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

// Amount of rainfall needed over 24hr period
extern int rainFallAmount; //(5 - 20mm)
// The time after it has stopped raining before the sprinklers can turn back on
extern int dryingTime; //(1 - 23 hrs)

void handleRoot() {
  char temp[100];

  // Handle any arguements passed in
  if (server.args() > 1) {
    String rainFallAmountStr = server.arg("rainFallAmount");
    int tempRainFall = rainFallAmountStr.toInt();
    if (tempRainFall > 4 && tempRainFall < 21) {
      rainFallAmount = tempRainFall;
    }
    String dryingTimeStr = server.arg("dryingTime");
    int tempDryingTime = dryingTimeStr.toInt();
    if (tempDryingTime > 0 && tempDryingTime < 24) {
      dryingTime = tempDryingTime;
    }
  }

  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send ( 200, "text/html", "" );

// Send HTML header
  server.sendContent(F("<html>\
  <head>\
    <title>WeatherFlow Rain Sensor</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
"));

// Send current configuation
server.sendContent(F("<body><h1>Configuation</h1><p><p><form><B>Rain fall needs to exceed <i>"));
server.sendContent(F("<select name=rainFallAmount>"));
for (int i = 5; i < 21; i++) {
  if (i == rainFallAmount)
    server.sendContent(F("<option selected>"));
  else
    server.sendContent(F("<option>"));
  sprintf(temp, "%d", i);
  server.sendContent(temp);
  server.sendContent(F("</option>"));
}
server.sendContent(F("</select>"));
server.sendContent(F("</i>mm over last 24hrs to trigger rain sensor</p><p> Rain must have dropped below trigger for <i>"));
server.sendContent(F("<select name=dryingTime>"));
for (int i = 1; i < 24; i++) {
  if (i == dryingTime)
    server.sendContent(F("<option selected>"));
  else
    server.sendContent(F("<option>"));
  sprintf(temp, "%d", i);
  server.sendContent(temp);
  server.sendContent(F("</option>"));
}
server.sendContent(F("</select>"));
server.sendContent(F("</i>hrs to reset rain sensor (drying time)</b></p>"));
server.sendContent(F("<input type=\"submit\" value=\"Update\"/></form> "));

// Send current status
server.sendContent(F("<hr><h1>Status</h1><p>Rain Sensor: "));
if (relayOffTime == -1) {
  server.sendContent(F("Not "));
}
server.sendContent(F("Triggered</p>"));
if (relayOffTime > 0) {
  server.sendContent(F("<p>Rain sensor will reset at: <span id='relayOffTime'></span></p>"));
  server.sendContent(F("<script> var date = new Date("));
  sprintf(temp, "%ld", relayOffTime);
  server.sendContent(temp);
  server.sendContent(F(" * 1000); var time = date.toLocaleTimeString();"));
  server.sendContent(F("document.getElementById('relayOffTime').innerHTML = time;</script>"));
}
server.sendContent(F("<p>Last time rain was detected: <span id='rainStartTime'></span></p>"));
server.sendContent(F("<script> var date = new Date("));
sprintf(temp, "%ld", rainStartEpoch);
server.sendContent(temp);
server.sendContent(F(" * 1000); var time = date.toLocaleTimeString(); var d = date.toDateString();"));
server.sendContent(F("document.getElementById('rainStartTime').innerHTML = d + ' ' + time;</script>"));
server.sendContent(F("<p>Rain fall total in last hour: "));
sprintf(temp, "%.2f", rainInHour[currentHour]);
server.sendContent(temp);
server.sendContent(F("mm</p><p>Rain fall total in last 24hrs: "));
float total = 0;
for (int hour = 0; hour < 24; hour++)
    total += rainInHour[hour];
sprintf(temp, "%.2f", total);
server.sendContent(temp);  
server.sendContent(F("mm</p><hr>"));

// Testing
// Send current status
server.sendContent(F("<h1>Testing</h1><p>Send Rain start event <button type=\"button\" onclick=\"rainstart()\">Now</button></p>"));
server.sendContent(F("<script>\
function rainstart()\
{\
    var xhr = new XMLHttpRequest();\
    xhr.open(\"GET\", \"/rainstart\", true); \
    xhr.send(null);\
}\
</script>"));

server.sendContent(F("<p>Send observation event with rain total 3.8 <button type=\"button\" onclick=\"obstest()\">Now</button> "));
server.sendContent(F("<script>\
function obstest()\
{\
    var xhr = new XMLHttpRequest();\
    xhr.open(\"GET\", \"/obstest\", true); \
    xhr.send(null);\
}\
</script>"));

server.sendContent(F("<p>Reset all stored rain data <button type=\"button\" onclick=\"reset()\">Reset</button> "));
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
  server.sendContent(F("</pre></p></body></html>"));
  server.client().stop();

}