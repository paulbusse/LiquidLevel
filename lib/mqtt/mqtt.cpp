#include <PubSubClient.h>
#include <Streaming.h>

#include <wifi.h>
#include <config.h>

#include "mqtt.h"

const uint32_t INIT_RETRY = 1 << 9; // 512 milliseconds
const uint32_t MAX_RETRY = 1 << 30;

const char * t0 = "/state";
const char * t1 = "/command";
const char * t2 = "/log";

const char * topicNames[] = { t0, t1, t2 };

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
MQTTClass MQTT;

MQTTClass::MQTTClass() : retry(INIT_RETRY) {

}

uint32_t MQTTClass::reconnect() {
    if( mqttClient.connected() ) {
        mqttClient.loop();
        retry = INIT_RETRY;
        return 0;
    }

    Serial << F("Setting up MQTT") << endl
           << F( "  Broker: ") << Config.mqttserver << endl
           << F( "  Port: ") << Config.mqttport << endl
           << F( "  ClientId: ") << clientId << endl;

    mqttClient.setServer(Config.mqttserver, Config.mqttport);
    if( mqttClient.connect( clientId.c_str() )) {
        retry = INIT_RETRY;
        Serial << F("Connected to ") << Config.mqttserver
               << F(" as ") << clientId << endl;
        return 0;
    }

    Serial << F("MQTT connect failed: ") << mqttClient.state() << endl;
    retry <<= 1;
    if( retry > MAX_RETRY)
        retry = MAX_RETRY;

    return retry;
}

boolean MQTTClass::publish( topic_t t, const char * buf) {
    return mqttClient.publish(topics[t], buf, true );
}

void MQTTClass::setup() {
    clientId = LLWifi.macaddress();
     for( int i = 0; i < 3; i++ ) {
        strcpy( topics[i], clientId.c_str());
        strncat( topics[i], topicNames[i], (size_t)32);
    }
   uint16_t keepAlive = (uint16_t)( Config.sonarinterval/500);
    mqttClient.setKeepAlive( keepAlive );

    (void)reconnect();
}

uint32_t MQTTClass::loop() {
    return reconnect();
}