#include <Arduino.h>
#include <Streaming.h>
#include <config.h>
#include <wifi.h>
#include <mqtt.h>

#define LED 2

void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);

  Serial.println( "Calling Config Setup");
  Config.setup();
  delay(5000);
  LLWifi.setup();
  delay(5000);
  MQTT.setup();
  delay(5000);
}

void loop() {

  digitalWrite(LED, HIGH);

  uint32_t ms = Config.loop();
  delay(5000);
  if( !ms ) ms = LLWifi.loop();
  delay(5000);
  if( !ms ) ms = MQTT.loop();
  delay(5000);

  if( !ms || ms > Config.sonarinterval)
    ms = Config.sonarinterval;
  
  Serial << F("Delaying ") << ms << F(" milliseconds.");

  digitalWrite(LED, LOW);

  delay( ms );
}
 