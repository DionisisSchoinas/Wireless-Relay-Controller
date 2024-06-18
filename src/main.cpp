#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <ESPAsyncWebServer.h> 

const char* ap_password = "Pa3s#Tz!a";

const char* device_hostname = "relays";

AsyncWebServer server(80); 

void setup() {
    // put your setup code here, to run once:
    Serial.begin(9600);
    
    //WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wm;
    wm.setHostname(device_hostname);
    WiFi.mode(WIFI_STA);

    // reset settings - wipe stored credentials for testing
    // these are stored by the esp library
    // wm.resetSettings();

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
        Serial.println("ESP32 Web Server: New request received:");  // for debugging 
        Serial.println("GET /");        // for debugging 
        request->send(200, "text/html", "<html><body><h1>Hello, ESP32!</h1></body></html>"); 
      }
    ); 

    // Start the server 
    server.begin();
}

void loop() {
}