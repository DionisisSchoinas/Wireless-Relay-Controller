#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <MyAsyncWebServer.h>

void handle_OnConnect(AsyncWebServerRequest *request);
void handle_led1on(AsyncWebServerRequest *request);
void handle_led1off(AsyncWebServerRequest *request);
void handle_led2on(AsyncWebServerRequest *request);
void handle_led2off(AsyncWebServerRequest *request);
void handle_NotFound(AsyncWebServerRequest *request);
String SendHTML(uint8_t led1stat, uint8_t led2stat);


const char *ap_password = "Pa3s#Tz!a";

const char *device_hostname = "relays";

MyAsyncWebServer server(80);

// uint8_t LED1pin = 4;
bool LED1status = LOW;

// uint8_t LED2pin = 5;
bool LED2status = LOW;

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
    server.on("/led1on", HTTP_GET, handle_led1on);
    server.on("/led1off", HTTP_GET, handle_led1off);
    server.on("/led2on", HTTP_GET, handle_led2on);
    server.on("/led2off", HTTP_GET, handle_led2off);
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
    Serial.print("GPIO4 Status: ");
    Serial.print(LED1status);
    Serial.print(" | GPIO5 Status: ");
    Serial.println(LED2status);
    request->send(200, "text/html", SendHTML(LED1status, LED2status));
}

void handle_led1on(AsyncWebServerRequest *request)
{
    LED1status = HIGH;
    Serial.println("GPIO4 Status: ON");
    request->send(200, "text/html", SendHTML(true, LED2status));
}

void handle_led1off(AsyncWebServerRequest *request)
{
    LED1status = LOW;
    Serial.println("GPIO4 Status: OFF");
    request->send(200, "text/html", SendHTML(false, LED2status));
}

void handle_led2on(AsyncWebServerRequest *request)
{
    LED2status = HIGH;
    Serial.println("GPIO5 Status: ON");
    request->send(200, "text/html", SendHTML(LED1status, true));
}

void handle_led2off(AsyncWebServerRequest *request)
{
    LED2status = LOW;
    Serial.println("GPIO5 Status: OFF");
    request->send(200, "text/html", SendHTML(LED1status, false));
}

void handle_NotFound(AsyncWebServerRequest *request)
{
    request->send(404, "text/plain", "Not found");
}

String SendHTML(uint8_t led1stat, uint8_t led2stat)
{
    String ptr = "<!DOCTYPE html> <html>\n";
    ptr += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
    ptr += "<title>LED Control</title>\n";
    ptr += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
    ptr += "body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
    ptr += ".button {display: block;width: 80px;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
    ptr += ".button-on {background-color: #3498db;}\n";
    ptr += ".button-on:active {background-color: #2980b9;}\n";
    ptr += ".button-off {background-color: #34495e;}\n";
    ptr += ".button-off:active {background-color: #2c3e50;}\n";
    ptr += "p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
    ptr += "</style>\n";
    ptr += "</head>\n";
    ptr += "<body>\n";
    ptr += "<h1>ESP32 Web Server</h1>\n";
    ptr += "<h3>Using Access Point(AP) Mode</h3>\n";

    if (led1stat)
    {
        ptr += "<p>LED1 Status: ON</p><a class=\"button button-off\" onclick=\"sendGet()\">OFF</a>\n";
    }
    else
    {
        ptr += "<p>LED1 Status: OFF</p><a class=\"button button-on\" onclick=\"sendGet()\">ON</a>\n";
    }

    if (led2stat)
    {
        ptr += "<p>LED2 Status: ON</p><a class=\"button button-off\" onclick=\"sendGet()\">OFF</a>\n";
    }
    else
    {
        ptr += "<p>LED2 Status: OFF</p><a class=\"button button-on\" onclick=\"sendGet()\">ON</a>\n";
    }

    ptr += "<script>function sendGet() {const xhttp = new XMLHttpRequest();xhttp.onload = function() {location.reload();};xhttp.open(\"GET\", \"http://relays/led1on\");xhttp.send();}</script>";
     
// <script>
// function sendGet() {
//   const xhttp = new XMLHttpRequest();
//   xhttp.onload = function() {
//     location.reload();
//   };
//   xhttp.open("GET", "http://relays/led1on");
//   xhttp.send();
// }
// </script>


    ptr += "</body>\n";
    ptr += "</html>\n";
    return ptr;
}