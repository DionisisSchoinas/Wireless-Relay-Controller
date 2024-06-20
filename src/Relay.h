#include "Arduino.h"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"

namespace httpsserver {
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
      void handleOn(HTTPRequest * req, HTTPResponse * res);
      void handleOff(HTTPRequest * req, HTTPResponse * res);
  };
}