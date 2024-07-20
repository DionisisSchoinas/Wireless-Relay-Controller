#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <Relay.h>
#include <EEPROM.h>
#include <esp_task_wdt.h>

// Includes for the server
#include <HTTPSServer.hpp>
#include <SSLCert.hpp>
#include <HTTPRequest.hpp>
#include <HTTPResponse.hpp>

// Include certificate data
// use lib/generateCa.sh to generate
#include "cert.h"
#include "private_key.h"

// The HTTPS Server comes in a separate namespace. For easier use, include it here.
using namespace httpsserver;

#define RELAY_NAME_SIZE 25  // Number of total characters (size of the char array) that can be saved in the config
#define MAIN_POWER_PIN A3
#define RELAY1_PIN 39
#define RELAY2_PIN 34
#define RELAY3_PIN 35
#define RELAY4_PIN 32

#define RED_LED 27
#define GREEN_LED 14
#define BLUE_LED 12

String generateHtml();
static Relay relay1(RELAY1_PIN, 1);
static Relay relay2(RELAY2_PIN, 2);
static Relay relay3(RELAY3_PIN, 3);
static Relay relay4(RELAY4_PIN, 4);

const char *ap_password = "Pa3s#Tz!a";
const char *device_hostname = "relays";

// Create an SSL certificate object from the files included above
SSLCert cert = SSLCert(
    myCert_crt_DER, myCert_crt_DER_len,
    myCert_key_DER, myCert_key_DER_len
);

// Create an SSL-enabled server that uses the certificate
HTTPSServer secureServer = HTTPSServer(&cert);


/*
*
*   Config related functions
*
*/
static const int CONFIG_EEPROM_ADDR = 0;

// configuration, as stored in EEPROM
struct Config {
    char relay1_name[RELAY_NAME_SIZE];
    char relay2_name[RELAY_NAME_SIZE];
    char relay3_name[RELAY_NAME_SIZE];
    char relay4_name[RELAY_NAME_SIZE];
    byte valid; // keep this as last byte
} config;

static void adjustConfigToDefaults() {
    strcpy(config.relay1_name, RELAY_DEFAULT_NAME);
    strcpy(config.relay2_name, RELAY_DEFAULT_NAME);
    strcpy(config.relay3_name, RELAY_DEFAULT_NAME);
    strcpy(config.relay4_name, RELAY_DEFAULT_NAME);
    // Always needed
    config.valid = 253;
}

static void loadFromEEPROM() {
    EEPROM.get(CONFIG_EEPROM_ADDR, config);

    if (config.valid != 253)
        adjustConfigToDefaults();
}

static void loadConfig() {
    loadFromEEPROM();

    // Update objects with new config
    relay1.setName(config.relay1_name);
    relay2.setName(config.relay2_name);
    relay3.setName(config.relay3_name);
    relay4.setName(config.relay4_name);
}

static bool isConfigValid() {
    if (strlen(config.relay1_name) > RELAY_NAME_SIZE)
        return false;
    if (strlen(config.relay2_name) > RELAY_NAME_SIZE)
        return false;
    if (strlen(config.relay3_name) > RELAY_NAME_SIZE)
        return false;
    if (strlen(config.relay4_name) > RELAY_NAME_SIZE)
        return false;

    return true;
}

// Returns true if successfully saved, otherwise returns false
static bool saveConfig() {
    if (!isConfigValid())
        return false;
    EEPROM.put(CONFIG_EEPROM_ADDR, config);
    EEPROM.commit();
    return true;
}

static bool saveConfigAndWasSaveSuccessful() {
    if (saveConfig()) {
        // re-init config
        loadConfig();
        return true;
    }
    // In case of problems get the values from EEPROM
    loadFromEEPROM();
    return false;
}


/*
*
*   Main power related functions
*
*/
static unsigned long lastMainPowerUpdate;
static bool mainPowerLastStatus;
static bool mainPowerOn;
void readMainPower() {
    mainPowerOn = analogRead(MAIN_POWER_PIN) > 300 ? true : false;
}

void updateMainPowerLastStatus() {
  if (mainPowerOn != mainPowerLastStatus && millis() >= lastMainPowerUpdate + 10000) {
    mainPowerLastStatus = mainPowerOn;
    lastMainPowerUpdate = millis();
  }
}


/*
*
*   HTTP request handler related functions
*
*/
void addRelayConfigHtml(Relay * relay, HTTPResponse * res) {
    res->println("<p>");
    res->print("Relay ");
    res->print(relay->getUrlNum());
    res->print(": ");
    res->print("<input id=\"relayInput");
    res->print(relay->getUrlNum());
    res->print("\" ");
    res->print("value=\"");
    res->print(relay->getName());
    res->println("\"/>");
    res->println("</p>");
}

void handleRoot(HTTPRequest * req, HTTPResponse * res) {
    res->setStatusCode(200);
    res->setHeader("Content-Type", "text/html");

    res->println("<!DOCTYPE html> <html>");
    res->println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">");
    res->println("<title>Relay Controller</title>");
    res->println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
    res->println("body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}");
    res->println(".button {display: block;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}");
    res->println(".button-small {display: block;border: none;color: white;padding: 10px 25px;text-decoration: none;font-size: 15px;margin: 0px auto 30px;cursor: pointer;border-radius: 3px;}");
    res->println(".button-on {background-color: #3498db;}");
    res->println(".button-on:hover {background-color: #2980b9;}");
    res->println(".button-on:disabled {background-color: #77b3db;}");
    res->println(".button-off {background-color: #34495e;}");
    res->println(".button-off:hover {background-color: #2c3e50;}");
    res->println(".button-off:disabled {background-color: #889199;}");
    res->println("p {font-size: 18px;color: #313131;margin-bottom: 10px;}");
    res->println("</style>");
    res->println("<style>");
    res->println(".mainPowerTable {margin-left: 50%}");
    res->println(".mainPowerTable > tbody > tr > td > div {display: flex; margin-left: -50%}");
    res->println(".mainPowerTable > tbody > tr > td > div > p {margin-top: 4px}");
    res->println(".mainPowerTable > tbody > tr > td > div > div {width: 30px;height: 30px;border-radius: 100px;margin-left: 10px;}");
    res->println(".green {background-color: #20d220}");
    res->println(".red {background-color: #ea0e0e}");
    res->println("</style>");
    res->println("</head>");
    res->println("<body>");
    
    res->print("<p>Your server has been running for<br><b style=\"font-size: 25px;\">");
    res->print((int)(millis()/60000), DEC);
    res->println("</b><br>minutes.</p>");

    res->println("<table class=\"mainPowerTable\">");
    res->println("<tr>");
    res->println("<td>");
    res->println("<div>");
    res->println("<p>Main Power: </p>");
    res->print("<div class=\"");
    res->print(mainPowerOn ? "green" : "red");
    res->print("\">");
    res->println("</div>");
    res->println("</div>");
    res->println("</td>");
    res->println("</tr>");
    res->println("</table>");
    res->println("<table class=\"mainPowerTable\">");
    res->println("<tr>");
    res->println("<td>");
    res->println("<div>");
    res->println("<p>Last Main Power: </p>");
    res->print("<div class=\"");
    res->print(mainPowerLastStatus ? "green" : "red");
    res->print("\">");
    res->println("</div>");
    res->println("</div>");
    res->println("</td>");
    res->println("</tr>");
    res->println("</table>");

    res->println("<table style=\"width: 50%;margin-left: 25%;\">");
    res->println("<tr>");
    res->println("<td>");
    res->println(relay1.getHtml());
    res->println("</td>");
    res->println("<td>");
    res->println(relay2.getHtml());
    res->println("</td>");
    res->println("</tr>");
    res->println("<tr>");
    res->println("<td>");
    res->println(relay3.getHtml());
    res->println("</td>");
    res->println("<td>");
    res->println(relay4.getHtml());
    res->println("</td>");
    res->println("</tr>");
    res->println("</table>");
    
    res->println("<div>");
    res->println("<h1 style=\"margin-top: 5px;\">Configurations</h1>");
    addRelayConfigHtml(&relay1, res);
    addRelayConfigHtml(&relay2, res);
    addRelayConfigHtml(&relay3, res);
    addRelayConfigHtml(&relay4, res);
    res->println("<button class=\"button button-on\" onclick=\"sendSave();this.disabled=true;\">Save Config</button>");
    res->println("<button class=\"button-small button-off\" onclick=\"sendReset();this.disabled=true;\">Reset Config</button>");
    res->println("</div>");
    
    res->println("<script>function sendGet(num, status) {const xhttp = new XMLHttpRequest();xhttp.onload = function() {location.reload();};xhttp.open(\"GET\", \"https://relays/relay\" + num + status);xhttp.send();}</script>");
    /*
    <script>
        function sendGet(num, status) {
            const xhttp = new XMLHttpRequest();
            xhttp.onload = function() {
                location.reload();
            };
            xhttp.open("GET", "https://relays/relay" + num + status);
            xhttp.send();
        }
    </script>
    */

    res->println("<script>function sendSave() {const xhttp = new XMLHttpRequest();xhttp.onload = function() {location.reload();};var url = \"https://relays/config?\";url += \"r1=\" + document.getElementById(\"relayInput1\").value + \"&\";url += \"r2=\" + document.getElementById(\"relayInput2\").value + \"&\";url += \"r3=\" + document.getElementById(\"relayInput3\").value + \"&\";url += \"r4=\" + document.getElementById(\"relayInput4\").value;xhttp.open(\"GET\", url);xhttp.send();}</script>");
    /*
    <script>
        function sendSave() {
            const xhttp = new XMLHttpRequest();
            xhttp.onload = function() {
                location.reload();
            };
            var url = "https://relays/config?";
            url += "r1=" + document.getElementById("relayInput1").value + "&";
            url += "r2=" + document.getElementById("relayInput2").value + "&";
            url += "r3=" + document.getElementById("relayInput3").value + "&";
            url += "r4=" + document.getElementById("relayInput4").value;
            xhttp.open("GET", url);
            xhttp.send();
        }
    </script>
    */

    res->println("<script>function sendReset() {const xhttp = new XMLHttpRequest();xhttp.onload = function() {location.reload();};xhttp.open(\"GET\", \"https://relays/config/reset\");xhttp.send();}</script>");
    /*
    <script>
        function sendReset() {
            const xhttp = new XMLHttpRequest();
            xhttp.onload = function() {
                location.reload();
            };
            xhttp.open("GET", "https://relays/config/reset");
            xhttp.send();
        }
    </script>
    */

    res->println("</body>");
    res->println("</html>");
}

void handle404(HTTPRequest * req, HTTPResponse * res) {
    // Discard request body, if we received any
    // We do this, as this is the default node and may also server POST/PUT requests
    req->discardRequestBody();

    // Set the response status
    res->setStatusCode(404);
    res->setStatusText("Not Found");

    // Set content type of the response
    res->setHeader("Content-Type", "text/html");

    // Write a tiny HTTP page
    res->println("<!DOCTYPE html>");
    res->println("<html>");
    res->println("<head><title>Not Found</title></head>");
    res->println("<body><h1>404 Not Found</h1><p>The requested resource was not found on this server.</p></body>");
    res->println("</html>");
}

void handleConfig(HTTPRequest * req, HTTPResponse * res) {
    ResourceParameters *params = req->getParams();
    for(auto it = params->beginQueryParameters(); it != params->endQueryParameters(); ++it) {
        if ((*it).first == "r1") {
            strcpy(config.relay1_name, (*it).second.c_str());
        } else if ((*it).first == "r2") {
            strcpy(config.relay2_name, (*it).second.c_str());
        } else if ((*it).first == "r3") {
            strcpy(config.relay3_name, (*it).second.c_str());
        } else if ((*it).first == "r4") {
            strcpy(config.relay4_name, (*it).second.c_str());
        }
    }

    if (saveConfigAndWasSaveSuccessful()) {
        res->setStatusCode(200);
        res->setHeader("Content-Type", "application/json");
        res->println("{\"status\":\"OK\"}");
        return;
    }

    res->setStatusCode(500);
    res->setHeader("Content-Type", "application/json");
    res->println("{\"status\":\"NOT_SAVED\"}");
}

void handleConfigReset(HTTPRequest * req, HTTPResponse * res) {
    adjustConfigToDefaults();
    if (saveConfigAndWasSaveSuccessful()) {
        res->setStatusCode(200);
        res->setHeader("Content-Type", "application/json");
        res->println("{\"status\":\"OK\"}");
        return;
    }

    res->setStatusCode(500);
    res->setHeader("Content-Type", "application/json");
    res->println("{\"status\":\"NOT_SAVED\"}");
}


/*
*
*   Main arduino related functions
*
*/
#define WDT_TIMEOUT 10

void setColor(int red, int green, int blue) {
    analogWrite(RED_LED, 255-red);
    analogWrite(GREEN_LED, 255-green);
    analogWrite(BLUE_LED, 255-blue);
}

void setupConfig() {
    EEPROM.begin(101);
    loadConfig();
    lastMainPowerUpdate = 0;
    mainPowerLastStatus = mainPowerOn;
}

void setup() {
    pinMode(RED_LED, OUTPUT);
    pinMode(GREEN_LED, OUTPUT);
    pinMode(BLUE_LED, OUTPUT);

    setColor(255, 0, 0);

    // put your setup code here, to run once:
    Serial.begin(9600);

    // WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wm;
    wm.setHostname(device_hostname);
    WiFi.mode(WIFI_STA);

    Serial.println("Starting up...");
    
    setColor(255, 255, 0);

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
    
    setColor(0, 255, 0);

    // Add the 404 not found node to the server.
    // The path is ignored for the default node.
    secureServer.setDefaultNode(new ResourceNode("", "GET", &handle404));
    // Add the root node to the server
    secureServer.registerNode(new ResourceNode("/", "GET", &handleRoot));
    // Config
    secureServer.registerNode(new ResourceNode("/config", "GET", &handleConfig));
    secureServer.registerNode(new ResourceNode("/config/reset", "GET", &handleConfigReset));
    // Relays
    secureServer.registerNode(new ResourceNode("/relay1on", "GET", [](HTTPRequest * req, HTTPResponse * res) { relay1.handleOn(req, res); }));
    secureServer.registerNode(new ResourceNode("/relay2on", "GET", [](HTTPRequest * req, HTTPResponse * res) { relay2.handleOn(req, res); }));
    secureServer.registerNode(new ResourceNode("/relay3on", "GET", [](HTTPRequest * req, HTTPResponse * res) { relay3.handleOn(req, res); }));
    secureServer.registerNode(new ResourceNode("/relay4on", "GET", [](HTTPRequest * req, HTTPResponse * res) { relay4.handleOn(req, res); }));
    secureServer.registerNode(new ResourceNode("/relay1off", "GET", [](HTTPRequest * req, HTTPResponse * res) { relay1.handleOff(req, res); }));
    secureServer.registerNode(new ResourceNode("/relay2off", "GET", [](HTTPRequest * req, HTTPResponse * res) { relay2.handleOff(req, res); }));
    secureServer.registerNode(new ResourceNode("/relay3off", "GET", [](HTTPRequest * req, HTTPResponse * res) { relay3.handleOff(req, res); }));
    secureServer.registerNode(new ResourceNode("/relay4off", "GET", [](HTTPRequest * req, HTTPResponse * res) { relay4.handleOff(req, res); }));

    Serial.println("Starting server...");
    secureServer.start();
    if (secureServer.isRunning()) {
        Serial.println("Server ready.");
    }

    setupConfig();
    
    pinMode(MAIN_POWER_PIN, INPUT);    

    esp_task_wdt_init(WDT_TIMEOUT, true); //enable panic so ESP32 restarts
    esp_task_wdt_add(NULL); //add current thread to WDT watch
    
    
    setColor(0, 0, 255);
}

void loop() {
    // This call will let the server do its work
    secureServer.loop();

    readMainPower();
    updateMainPowerLastStatus();
    
    esp_task_wdt_reset();
}