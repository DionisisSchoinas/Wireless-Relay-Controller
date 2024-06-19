#include "Arduino.h"
#include "Relay.h"

Relay::Relay(int relayPin, int urlNum) {
  this-> relayPin = relayPin;
  // pinMode(relayPin, OUTPUT);
  setToOn(false);
  this-> urlNum = urlNum;
  this-> name = "N/A";
}

void Relay::setPinState(int state) {
  // digitalWrite(relayPin, state);
}

bool Relay::isOn() {
  return on;
}

int Relay::getRelayPin() {
  return relayPin;
}

int Relay::getUrlNum() {
  return urlNum;
}

void Relay::setToOn(bool turnOn) {
  this-> on = turnOn;
  setPinState(turnOn);
}

String Relay::getName() {
  return name;
}

void Relay::setName(String name) {
  this-> name = name;
}

String Relay::getHtml() {
  /* Generates something like this
    <p>
      Relay Name 1 Status: ON
    </p>
    <button class=\"button button-off\" onclick="sendGet(1, 'off')">
      OFF
    </button>
  */

  String html = "<p><b>";
  html += getName();
  html += "</b> Status: ";
  html += isOn() ? "ON" : "OFF";
  html += "</p><button class=\"button ";
  html += isOn() ? "button-off" : "button-on";
  html += "\" onclick=\"sendGet(";
  html += getUrlNum();
  html += ", ";
  html += isOn() ? "'off'" : "'on'";
  html += ")\">";
  html += isOn() ? "Turn OFF" : "Turn ON";
  html += "</button>";
  return html;
};

void Relay::handle_on(AsyncWebServerRequest *request)
{
  setToOn(true);
  request->send(200, "application/json", "{\"status\":\"OK\"}");
}

void Relay::handle_off(AsyncWebServerRequest *request)
{
  setToOn(false);
  request->send(200, "application/json", "{\"status\":\"OK\"}");
}