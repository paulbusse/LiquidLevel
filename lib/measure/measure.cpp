#include <stdint.h>
#include <Streaming.h>

#include <pins_arduino.h>
#include "mqtt.h"
#include "measure.h"

const uint32_t INIT_RETRY = 1 << 9; // 512 milliseconds
const uint32_t MAX_RETRY = 1 << 30;

const uint8_t TRIGGER = D2;
const uint8_t ECHO = D1;

// Sound travels 340m/s.
// Duration is in µs => 0,000340 m/µs
// We are interested in cm => 0,034cm/µs
// We are measuring twice the length (away and back) => 0.017cm/µs
const float CMpMICROs = 0.017;

MeasureClass Measure;

MeasureClass::MeasureClass() : retry(INIT_RETRY) {
    // empty
}

void MeasureClass::setup() {
    pinMode(TRIGGER, OUTPUT);
    pinMode(ECHO, INPUT);
}

uint32_t MeasureClass::_publish(float distance) {
  char buff[64];
  snprintf_P( buff, sizeof(buff) - 1, PSTR("distance: %.2f"), distance );
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

const int nSamples = 10;
uint32_t MeasureClass::loop() {
    uint32_t durations[nSamples];
    uint32_t sum = 0;
    float average;
    float stdev;
    float distance;

    Serial.println("Start measuring");
    digitalWrite(TRIGGER, LOW);   // Makes trigPin low
    delayMicroseconds(2);       // 2 micro second delay 

    // Collecting 10 measurements
    for( int i = 0; i < nSamples; i ++) {
        digitalWrite(TRIGGER, HIGH);  // tigPin high
        delayMicroseconds(10);      // trigPin high for 10 micro seconds
        digitalWrite(TRIGGER, LOW);   // trigPin low

        durations[i] = pulseIn(ECHO, HIGH);   //Read echo pin, time in microseconds
        sum += durations[i];
        Serial << F("Measured duration") << durations[i] << endl;
        delay(1000);
    }
    
    average = (float)sum / (float) nSamples;
    stdev = 0;
    for( int i = 0; i < 10; i ++) {
        stdev += (durations[i] - average) * (durations[i] - average);
    }
    stdev = sqrt( stdev/(nSamples - 1.0) );

    if( stdev > 20.0 ) {
        Serial << F("Issues with the samples. Ignoring samples.") << endl;
        retry <<= 1;
        if( retry > MAX_RETRY)
            retry = MAX_RETRY;
        return retry;
    }

    distance = average * CMpMICROs;        //Calculating actual/real distance
    retry = INIT_RETRY;

    Serial.print("Distance = ");        //Output distance on arduino serial monitor 
    Serial.println(distance);
    _publish(distance);
    return 0;
}