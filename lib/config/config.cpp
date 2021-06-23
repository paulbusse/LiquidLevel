#include <float.h>
#include <Arduino.h>
#include <Streaming.h>
#include <LittleFS.h>
#include <__secret.h>
#include "config.h"

ConfigClass Config;
DynamicJsonDocument ConfigClass::json(CONFIGLENGTH);

const uint32_t SEC = 1000;
const uint32_t MIN = 60 * SEC;
const uint32_t HOUR = 60 * MIN;
const uint32_t DAY = 24 * HOUR;

struct configconfig_t {
    const char * name;
    union {
        struct strconfig {
            const char * dflt;
        } sc;
        struct intconfig {
            uint32_t min;
            uint32_t max;
            uint32_t dflt;
        } ic;
        struct floatconfig {
            float min;
            float max;
            float dflt;
        } fc;
    } u;
} cc[] = {
    { "wifissid", { .sc = {SECRET_SSID} } },
    { "wifipwd", { .sc = {SECRET_PWD} }  },
    { "mqttserver", { .sc = {SECRET_BROKER} }  },
    { "mqttport", {.ic = { 1024, 65535, 1883 } } },
    { "sonardepth", {.ic = { 20, 500, 100 } } },
    { "sonarcount", {.ic = { 1, 100, 10 } } },
    { "sonarinterval", {.ic = { 1 * SEC, 32767 * SEC, 10 * SEC } } },
    { "sonarlpercm", {.fc = { 0.01, FLT_MAX, 1.0 } } }
};

ConfigClass::ConfigClass()
{
    // empty
}

void ConfigClass::_config( configfield_t configfield, const char ** attrib ) {
    const char * field = cc[configfield].name;
    const char * dflt = cc[configfield].u.sc.dflt;
    Serial.println( dflt );

    if( json[field].isNull() )
        *attrib = dflt;
    else
        *attrib = json[field];
    Serial << field << F(": ") << ** attrib << endl;
}

void ConfigClass::_config( configfield_t configfield, uint16_t *attrib) {
    const char * field = cc[configfield].name;
    uint16_t min = cc[configfield].u.ic.min;
    uint16_t max = cc[configfield].u.ic.max;
    uint16_t dflt = cc[configfield].u.ic.dflt;
    if( json[field].isNull() )
        *attrib = dflt;
    else {
        int32_t jfield = json[field];
        if( jfield < min ) *attrib = min;
        else if( jfield > max ) *attrib = max;
        else *attrib = jfield;
    }
    Serial << field << F(": ") << * attrib << endl;
}

void ConfigClass::_config( configfield_t configfield, float *attrib){
    const char * field = cc[configfield].name;
    float min = cc[configfield].u.fc.min;
    float max = cc[configfield].u.fc.max;
    float dflt = cc[configfield].u.fc.dflt;

    if( json[field].isNull() )
        *attrib = dflt;
    else {
        float jfield = json[field];
        if( jfield < min ) *attrib = min;
        else if( jfield > max ) *attrib = max;
        else *attrib = jfield;
    }
    Serial << field << F(": ") << * attrib << endl;
}

void ConfigClass::_cfginterval( configfield_t configfield, uint32_t *attrib){
    const char * field = cc[configfield].name;
    uint32_t min = cc[configfield].u.ic.min;
    uint32_t max = cc[configfield].u.ic.max;
    uint32_t dflt = cc[configfield].u.ic.dflt;

    if( json[field].isNull() )
        *attrib = dflt;
    else {
        const char * s = json[field];
        uint32_t i32 = json[field];
        switch( s[strlen(s) - 1]) {
            case 's': i32 *= SEC; break;
            case 'm': i32 *= MIN; break;
            case 'h': i32 *= HOUR; break;
            default: /* No-op */ break;
        }
        if( i32 < min ) *attrib = min;
        else if( i32 > max ) *attrib = max;
        else *attrib = i32;
    }
    Serial << field << F(": ") << * attrib << endl;
}

void ConfigClass::setup() {
    Serial << F("Configuring ...") << endl;
    const char * input = "{\"production\": 0 }";
    /*LittleFS.begin();

    File cfgfile = LittleFS.open("/config.json", "r");
    */
    DeserializationError error = deserializeJson(json, input, strlen(input));
    if (error)
        Serial << F("Failed to read file, using default configuration") << endl;
    /*
    cfgfile.close();
    // LittleFS.end(); // We keep it unmounted unless needed
    */

    _config(WIFISSID, & wifissid );
    _config(WIFIPWD, &wifipwd);
    _config(MQTTSERVER, &mqttserver);
    _config(MQTTPORT, &mqttport);
    _config(SONARDEPTH, &sonardepth);
    _config(SONARCOUNT, &sonarcount);
    _config(SONARLPERCM, &sonarlpercm);

    _cfginterval(SONARINTERVAL, &sonarinterval);
    return;
}

uint32_t ConfigClass::loop() {
    return 0; // 0 is not causing a delay.
}