#include <ESPAsyncWebServer.h>

// Wrapper class to access the protected server status
class MyAsyncWebServer : public AsyncWebServer {
    public:
        MyAsyncWebServer(uint16_t port);

        uint8_t getStatus() {
            return AsyncWebServer::_server.status();
        }
};