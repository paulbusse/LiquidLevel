#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <ArduinoJson.h>

enum configfield_t {
    WIFISSID = 0,
    WIFIPWD,
    MQTTSERVER,
    MQTTPORT,
    SONARDEPTH,
    SONARCOUNT,
    SONARINTERVAL,
    SONARLPERCM
};

const int CONFIGLENGTH = 1024;

class ConfigClass {
    private:
        static DynamicJsonDocument json;

        void _config( configfield_t, const char **);
        void _config( configfield_t, uint16_t *);
        void _config( configfield_t, float *);
        void _cfginterval( configfield_t, uint32_t *);

    public:
        const char * wifissid;
        const char * wifipwd;
        const char * mqttserver;
        float        sonarlpercm;
        uint32_t     sonarinterval;
        uint16_t     mqttport;
        uint16_t     sonarcount;
        uint16_t     sonardepth;

        ConfigClass();
        void setup();
        uint32_t loop();

};

extern class ConfigClass Config;
#endif // __CONFIG_H__