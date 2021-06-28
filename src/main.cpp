#include <Arduino.h>
#include <Streaming.h>
#include <config.h>
#include <wifi.h>
#include <mqtt.h>
#include <measure.h>

#define LED 2

const uint32_t INIT_RETRY = 1 << 9; // +-0.5 second
const uint32_t MAX_RETRY = 1 << 30;

uint32_t retry = INIT_RETRY;

void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);

  Serial.println( "Calling Config Setup");
  Config.setup();
  LLWifi.setup();
  MQTT.setup();
  Measure.setup();
}

void loop() {

  digitalWrite(LED, HIGH);

  uint32_t ms = Config.loop();
  if( !ms ) ms = LLWifi.loop();
  if( !ms ) ms = MQTT.loop();
  if( !ms ) ms = Measure.loop();
  
  if( !ms || ms > Config.sonarinterval)
    ms = Config.sonarinterval;
  
  Serial << F("Delaying ") << ms << F(" milliseconds.");

  digitalWrite(LED, LOW);

  delay( ms );
}
 