#include <Arduino.h>
#include <Streaming.h>
#include <config.h>
#include <wifi.h>
#include <mqtt.h>

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
}

uint32_t do_publish() {
  char buff[64];
  snprintf_P( buff, sizeof(buff) - 1, PSTR("Millis: %ld"), millis() );
  if( MQTT.publish( STATE_TOPIC, buff) ) {
      Serial << F("Publish succeeded.") << endl;
      retry = INIT_RETRY;
      return 0;
  }

  Serial << F("Publish failed") << endl;
  retry <<= 1;
  if( retry > MAX_RETRY) retry = MAX_RETRY;
  return retry;
}

void loop() {

  digitalWrite(LED, HIGH);

  uint32_t ms = Config.loop();
  if( !ms ) ms = LLWifi.loop();
  if( !ms ) ms = MQTT.loop();
  if( !ms ) ms = do_publish();
  
  if( !ms || ms > Config.sonarinterval)
    ms = Config.sonarinterval;
  
  Serial << F("Delaying ") << ms << F(" milliseconds.");

  digitalWrite(LED, LOW);

  delay( ms );
}
 