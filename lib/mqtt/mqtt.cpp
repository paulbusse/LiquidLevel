#include <PubSubClient.h>
#include <Streaming.h>

#include <wifi.h>
#include <config.h>

#include "mqtt.h"

const uint32_t INIT_RETRY = 1 << 9; // 512 milliseconds
const uint32_t MAX_RETRY = 1 << 30;

const char * t0 PROGMEM = "/state";
const char * t1 PROGMEM = "/command";
const char * t2 PROGMEM = "/log";

const char * topicNames[] PROGMEM = { t0, t1, t2 };

MQTTClass MQTT;
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

MQTTClass::MQTTClass() : retry(INIT_RETRY) {
    clientId = LLWifi.macaddress();
    mqttClient.setKeepAlive( (uint16_t) Config.sonarinterval/1000 );

    for( int i = 0; i < 3; i++ ) {
        strcpy( topics[i], clientId.c_str());
        strncat_P(topics[i], (const char*)pgm_read_dword(&(topicNames[i])), (size_t)32);
    }
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
    (void)reconnect();
}

uint32_t MQTTClass::loop() {
    return reconnect();
    char buff[64];
    snprintf_P( buff, sizeof(buff) - 1, PSTR("Millis: %ld"), millis() );
    if( publish( STATE_TOPIC, buff) )
        Serial << F("Publish succeeded.") << endl;
    else
        Serial << F("Publish failed") << endl;
}