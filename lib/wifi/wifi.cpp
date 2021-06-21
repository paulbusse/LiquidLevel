#include <Streaming.h>
#include <ESP8266WiFi.h>

#include <config.h>

#include "wifi.h"

const uint32_t INIT_RETRY = 1 << 9; // +-0.5 second
const uint32_t MAX_RETRY = 1 << 30;  

WifiClass LLWifi;

WifiClass::WifiClass() : retry(INIT_RETRY) {
    // empty
}

void WifiClass::setup() {

    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
   
    (void) reconnect();
}

// Returns the time until we retry again
uint32_t WifiClass::reconnect() {
    wl_status_t status = WiFi.status();
    if( status == WL_CONNECTED ) {
        retry = INIT_RETRY;
        return 0;
    }

    // Set up SSID and password whenever it changes
    if( WiFi.SSID() != Config.wifissid || WiFi.psk() != Config.wifipwd )
        WiFi.begin(Config.wifissid, Config.wifipwd);
    else
        WiFi.begin(); 

    if( WiFi.waitForConnectResult() == WL_CONNECTED) {
        retry = INIT_RETRY;
        Serial << F("Wifi connected:") << endl
               << F("  IP Address: ") << WiFi.localIP() << endl
               << F("  MAC Address: ") << WiFi.macAddress() << endl;
        return 0;
    }
    
    // could not connect
    retry <<= 1;
    if ( retry > MAX_RETRY )
        retry = MAX_RETRY;

    Serial << F("Wifi connection failed ... retrying") << endl;
    return retry;
}

uint32_t WifiClass::loop() {
    return reconnect();
}