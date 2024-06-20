#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <Relay.h>

// void handle_relay1On(AsyncWebServerRequest *request);
// void handle_relay2On(AsyncWebServerRequest *request);
// void handle_relay3On(AsyncWebServerRequest *request);
// void handle_relay4On(AsyncWebServerRequest *request);
// void handle_relay1Off(AsyncWebServerRequest *request);
// void handle_relay2Off(AsyncWebServerRequest *request);
// void handle_relay3Off(AsyncWebServerRequest *request);
// void handle_relay4Off(AsyncWebServerRequest *request);
// void handle_OnConnect(AsyncWebServerRequest *request);
// void handle_NotFound(AsyncWebServerRequest *request);
String generateHtml();

static Relay relay1(1, 1);
static Relay relay2(2, 2);
static Relay relay3(3, 3);
static Relay relay4(4, 4);

const char *ap_password = "Pa3s#Tz!a";
const char *device_hostname = "relays";

// Include certificate data
// use lib/generateCa.sh to generate
#include "cert.h"
#include "private_key.h"

// Includes for the server
#include <HTTPSServer.hpp>
#include <SSLCert.hpp>
#include <HTTPRequest.hpp>
#include <HTTPResponse.hpp>

// The HTTPS Server comes in a separate namespace. For easier use, include it here.
using namespace httpsserver;

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

    // For every resource available on the server, we need to create a ResourceNode
    // The ResourceNode links URL and HTTP method to a handler function
    ResourceNode * nodeRoot    = new ResourceNode("/", "GET", &handleRoot);
    ResourceNode * node404     = new ResourceNode("", "GET", &handle404);

    // Add the root node to the server
    secureServer.registerNode(nodeRoot);

    // Add the 404 not found node to the server.
    // The path is ignored for the default node.
    secureServer.setDefaultNode(node404);

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
  // Status code is 200 OK by default.
  // We want to deliver a simple HTML page, so we send a corresponding content type:
  res->setHeader("Content-Type", "text/html");

  // The response implements the Print interface, so you can use it just like
  // you would write to Serial etc.
  res->println("<!DOCTYPE html>");
  res->println("<html>");
  res->println("<head><title>Hello World!</title></head>");
  res->println("<body>");
  res->println("<h1>Hello World!</h1>");
  res->print("<p>Your server is running for ");
  // A bit of dynamic data: Show the uptime
  res->print((int)(millis()/1000), DEC);
  res->println(" seconds.</p>");
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