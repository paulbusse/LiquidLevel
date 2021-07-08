#include <cfloat>
#include <Arduino.h>
#include <Streaming.h>
#include <LittleFS.h>
#include <__secret.h>
#include "config.h"

// The json part is here for future enhancements
// That should allow configs to be updated over MQTT

const char * path = "/config2.json";

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
    { "sonarinterval", {.ic = { 1 * SEC, 32767 * SEC, 30 * SEC } } },
    { "sonarlpercm", {.fc = { 0.01, FLT_MAX, 1.0 } } }
};

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
        int32_t value = json[field];
        if( value < min ) *attrib = min;
        else if( value > max ) *attrib = max;
        else *attrib = value;
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
        float value = json[field];
        if( value < min ) *attrib = min;
        else if( value > max ) *attrib = max;
        else *attrib = value;
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
        Serial << "cfginterval: " << json[field].as<const char *>() << endl;
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

E_config_t ConfigClass::_getConfig( char * buf, size_t n) {
     Serial << F("Start _getconfig") << endl;

    if( !LittleFS.exists(path) )
        return E_NOTEXISTS;

    File configFile = LittleFS.open(path, "r");
    if (!configFile)
        return E_OPEN;

    size_t size = configFile.size();
    if (size > n)
        return E_SIZE;

    Serial << "Getconfig: " << buf << endl;
    configFile.readBytes(buf, size);
    configFile.close();

    auto error = deserializeJson(json, buf);
    if (error) {
        Serial.println("Failed to parse config file. Removing file.");
        LittleFS.remove(path);
        return E_NOTEXISTS;
    }
 Serial << F("End _getconfig") << buf << endl;
    return E_NONE;
}

E_config_t ConfigClass::_saveConfig() {

    Serial << F("Start _saveconfig") << endl;

  File configFile = LittleFS.open(path, "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return E_OPEN;
  }
  json[WIFISSID] = wifissid;
  json[WIFIPWD] = wifipwd;
  json[MQTTSERVER] = mqttserver;
  json[MQTTPORT] = mqttport;
  json[SONARDEPTH] = sonardepth;
  json[SONARCOUNT] = sonarcount;
  json[SONARLPERCM] = sonarlpercm;
  serializeJson(json, configFile);

  configFile.close();
  Serial << F("End _saveconfig") << endl;
  return E_NONE;
}


void ConfigClass::setup() {
    char buf[CONFIGLENGTH];

    Serial << F("Setup config.") << endl;

    LittleFS.begin();

    E_config_t ect =_getConfig(buf, CONFIGLENGTH);

    Serial << F("Getconfig returned") << ect << endl;
    if( ect != E_NONE )
        Serial << F("No configuration file found. Using defaults") << endl;

    _config(WIFISSID, & wifissid );
    _config(WIFIPWD, &wifipwd);
    _config(MQTTSERVER, &mqttserver);
    _config(MQTTPORT, &mqttport);
    _config(SONARDEPTH, &sonardepth);
    _config(SONARCOUNT, &sonarcount);
    _config(SONARLPERCM, &sonarlpercm);

    _cfginterval(SONARINTERVAL, &sonarinterval);

    if( ect == E_NOTEXISTS )
        _saveConfig();
    
    Serial << F("Setup config. Done.") << endl;

    return;
}

uint32_t ConfigClass::loop() {
    return 0; // 0 is not causing a delay.
}