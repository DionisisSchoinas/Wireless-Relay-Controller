#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <MyAsyncWebServer.h>

const char* ap_password = "Pa3s#Tz!a";

const char* device_hostname = "relays";

MyAsyncWebServer server(80); 

void setup() {
    // put your setup code here, to run once:
    Serial.begin(9600);
    
    //WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wm;
    wm.setHostname(device_hostname);
    WiFi.mode(WIFI_STA);

    Serial.println("Starting up...");

    bool res;
    res = wm.autoConnect(wm.getDefaultAPName().c_str(), ap_password); // password protected ap

    if(!res) {
        Serial.println("Failed to connect");
        ESP.restart();
    } 
    else {
        //if you get here you have connected to the WiFi    
        Serial.println("connected...yeey :)");
    }

    // Define a route to serve the HTML page 
    server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(200, "text/html", "<html><body><h1>Hello, ESP32!</h1></body></html>"); 
      }
    );

    // Start the server 
    server.begin();

    // Check if server started successfully
    if (server.getStatus() != 1) {
        Serial.println("Failed to start HTTP server !!!");
        Serial.println("Restarting...");
        ESP.restart();
    }
}

void loop() {
}