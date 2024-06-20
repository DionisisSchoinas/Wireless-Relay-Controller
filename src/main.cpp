#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <Relay.h>
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

String generateHtml();

static Relay relay1(1, 1);
static Relay relay2(2, 2);
static Relay relay3(3, 3);
static Relay relay4(4, 4);

const char *ap_password = "Pa3s#Tz!a";
const char *device_hostname = "relays";

// MyAsyncWebServer server(80);

// Create an SSL certificate object from the files included above
SSLCert cert = SSLCert(
  myCert_crt_DER, myCert_crt_DER_len,
  myCert_key_DER, myCert_key_DER_len
);

// Create an SSL-enabled server that uses the certificate
// The contstructor takes some more parameters, but we go for default values here.
HTTPSServer secureServer = HTTPSServer(&cert);

// Declare some handler functions for the various URLs on the server
// The signature is always the same for those functions. They get two parameters,
// which are pointers to the request data (read request body, headers, ...) and
// to the response data (write response, set status code, ...)
void handleRoot(HTTPRequest * req, HTTPResponse * res);
void handleFavicon(HTTPRequest * req, HTTPResponse * res);
void handle404(HTTPRequest * req, HTTPResponse * res);

void setup() {
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


    // Add the 404 not found node to the server.
    // The path is ignored for the default node.
    secureServer.setDefaultNode(new ResourceNode("", "GET", &handle404));
    // Add the root node to the server
    secureServer.registerNode(new ResourceNode("/", "GET", &handleRoot));
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
}

void loop() {
  // This call will let the server do its work
  secureServer.loop();

  // Other code would go here...
  delay(1);
}

void handleRoot(HTTPRequest * req, HTTPResponse * res) {
    res->setStatusCode(200);
    res->setHeader("Content-Type", "text/html");

    res->println("<!DOCTYPE html> <html>");
    res->println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">");
    res->println("<title>Relay Controller</title>");
    res->println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
    res->println("body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}");
    res->println(".button {display: block;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}");
    res->println(".button-on {background-color: #3498db;}");
    res->println(".button-on:hover {background-color: #2980b9;}");
    res->println(".button-on:disabled {background-color: #77b3db;}");
    res->println(".button-off {background-color: #34495e;}");
    res->println(".button-off:hover {background-color: #2c3e50;}");
    res->println(".button-off:disabled {background-color: #889199;}");
    res->println("p {font-size: 18px;color: #313131;margin-bottom: 10px;}");
    res->println("</style>");
    res->println("</head>");
    res->println("<body>");
    
    res->print("<p>Your server is running for<br><b>");
    res->print((int)(millis()/60000), DEC);
    res->println("</b><br>minutes.</p>");

    res->println(relay1.getHtml());
    res->println(relay2.getHtml());
    res->println(relay3.getHtml());
    res->println(relay4.getHtml());
    
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