#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <MyAsyncWebServer.h>
#include <Relay.h>

void handle_relay1On(AsyncWebServerRequest *request);
void handle_relay2On(AsyncWebServerRequest *request);
void handle_relay3On(AsyncWebServerRequest *request);
void handle_relay4On(AsyncWebServerRequest *request);
void handle_relay1Off(AsyncWebServerRequest *request);
void handle_relay2Off(AsyncWebServerRequest *request);
void handle_relay3Off(AsyncWebServerRequest *request);
void handle_relay4Off(AsyncWebServerRequest *request);
void handle_OnConnect(AsyncWebServerRequest *request);
void handle_NotFound(AsyncWebServerRequest *request);
String generateHtml();

static Relay relay1(1, 1);
static Relay relay2(2, 2);
static Relay relay3(3, 3);
static Relay relay4(4, 4);

const char *ap_password = "Pa3s#Tz!a";
const char *device_hostname = "relays";

MyAsyncWebServer server(80);

void setup()
{
    // put your setup code here, to run once:
    Serial.begin(9600);

    // WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wm;
    wm.setHostname(device_hostname);
    WiFi.mode(WIFI_STA);

    Serial.println("Starting up...");

    bool res;
    res = wm.autoConnect(wm.getDefaultAPName().c_str(), ap_password); // password protected ap

    if (!res)
    {
        Serial.println("Failed to connect");
        ESP.restart();
    }
    else
    {
        // if you get here you have connected to the WiFi
        Serial.println("connected...yeey :)");
    }

    server.on("/", HTTP_GET, handle_OnConnect);
    server.on("/relay1on", HTTP_GET, handle_relay1On);
    server.on("/relay1off", HTTP_GET, handle_relay1Off);
    server.on("/relay2on", HTTP_GET, handle_relay2On);
    server.on("/relay2off", HTTP_GET, handle_relay2Off);
    server.on("/relay3on", HTTP_GET, handle_relay3On);
    server.on("/relay3off", HTTP_GET, handle_relay3Off);
    server.on("/relay4on", HTTP_GET, handle_relay4On);
    server.on("/relay4off", HTTP_GET, handle_relay4Off);
    server.onNotFound(handle_NotFound);

    // Start the server
    server.begin();

    // Check if server started successfully
    if (server.getStatus() != 1)
    {
        Serial.println("Failed to start HTTP server !!!");
        Serial.println("Restarting...");
        ESP.restart();
    }
}

void loop()
{
    //   if(LED1status)
    //   {digitalWrite(LED1pin, HIGH);}
    //   else
    //   {digitalWrite(LED1pin, LOW);}

    //   if(LED2status)
    //   {digitalWrite(LED2pin, HIGH);}
    //   else
    //   {digitalWrite(LED2pin, LOW);}
}

void handle_OnConnect(AsyncWebServerRequest *request)
{
    request->send(200, "text/html", generateHtml());
}

void handle_NotFound(AsyncWebServerRequest *request)
{
    request->send(404, "text/plain", "Not found");
}

void handle_relay1On(AsyncWebServerRequest *request)
{
    relay1.handle_on(request);
}

void handle_relay2On(AsyncWebServerRequest *request)
{
    relay2.handle_on(request);
}

void handle_relay3On(AsyncWebServerRequest *request)
{
    relay3.handle_on(request);
}

void handle_relay4On(AsyncWebServerRequest *request)
{
    relay4.handle_on(request);
}

void handle_relay1Off(AsyncWebServerRequest *request)
{
    relay1.handle_off(request);
}

void handle_relay2Off(AsyncWebServerRequest *request)
{
    relay2.handle_off(request);
}

void handle_relay3Off(AsyncWebServerRequest *request)
{
    relay3.handle_off(request);
}

void handle_relay4Off(AsyncWebServerRequest *request)
{
    relay4.handle_off(request);
}

String generateHtml()
{
    String ptr = "<!DOCTYPE html> <html>\n";
    ptr += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
    ptr += "<title>Relay Controller</title>\n";
    ptr += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
    ptr += "body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
    ptr += ".button {display: block;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
    ptr += ".button-on {background-color: #3498db;}\n";
    ptr += ".button-on:active {background-color: #2980b9;}\n";
    ptr += ".button-off {background-color: #34495e;}\n";
    ptr += ".button-off:active {background-color: #2c3e50;}\n";
    ptr += "p {font-size: 18px;color: #313131;margin-bottom: 10px;}\n";
    ptr += "</style>\n";
    ptr += "</head>\n";
    ptr += "<body>\n";
    ptr += "<h1>ESP32 Relay Controller</h1>\n";

    ptr += relay1.getHtml();
    ptr += relay2.getHtml();
    ptr += relay3.getHtml();
    ptr += relay4.getHtml();
    
    ptr += "<script>function sendGet(num, status) {const xhttp = new XMLHttpRequest();xhttp.onload = function() {location.reload();};xhttp.open(\"GET\", \"http://relays/relay\" + num + status);xhttp.send();}</script>";
    /*
    <script>
    function sendGet(num, status) {
        const xhttp = new XMLHttpRequest();
        xhttp.onload = function() {
            location.reload();
        };
        xhttp.open("GET", "http://relays/relay" + num + status);
        xhttp.send();
    }
    </script>
    */

    ptr += "</body>\n";
    ptr += "</html>\n";
    return ptr;
}