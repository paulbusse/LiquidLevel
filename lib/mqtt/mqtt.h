#ifndef __MQTT_H__
#define __MQTT_H__

#include <PubSubClient.h>

enum topic_t {
    STATE_TOPIC = 0,
    COMMAND_TOPIC,
    LOG_TOPIC,
};

class MQTTClass {
    private:
        String clientId;
        uint32_t retry;
        char topics[3][32];

        uint32_t reconnect();

    public:
             MQTTClass();
        void setup();
        uint32_t loop();
        boolean publish(topic_t, const char *);
};

extern MQTTClass MQTT;
#endif // __MQTT_H__