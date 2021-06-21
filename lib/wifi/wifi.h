#ifndef __WIFI_H__
#define __WIFI_H__

#include <ESP8266WiFi.h>

class WifiClass {
    private:
        uint32_t retry;
        uint32_t reconnect();

    public:
             WifiClass();
        void setup();
        uint32_t loop();
        String macaddress() {
            return WiFi.macAddress();
        }
};

extern WifiClass LLWifi;
#endif // __WIFI_H__