#include "Arduino.h"
#include "ESPAsyncWebServer.h"

class Relay {
  private:
    uint relayPin;
    String name;
    uint urlNum;
    bool on;
    void setPinState(int state);

  public:
    Relay(int relayPin, int urlNum);
    int getRelayPin();
    bool isOn();
    void setToOn(bool turnOn);
    int getUrlNum();
    String getName();
    void setName(String name);
    String getHtml();
    void handle_on(AsyncWebServerRequest *request);
    void handle_off(AsyncWebServerRequest *request);
};