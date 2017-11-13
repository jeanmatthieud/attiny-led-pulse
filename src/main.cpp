#include <Arduino.h>
#include <avr/sleep.h>

#define PIN_LED 1
#define PIN_SENSOR 0
#define PCIE_SENSOR PCINT0
#define NUDGE_PULSE_DURATION 1500L
#define NUDGE_DURATION NUDGE_PULSE_DURATION * 5

enum State {
  BOOT,
  POWER_DOWN,
  NUDGE_START,
  NUDGE_PROGRESS,
  NUDGE_STOP
};

volatile State g_currentState = BOOT;
long g_nudgeStartMillis = 0L;

ISR(PCINT0_vect) {
  // High level and currently in power down mode (software rising edge)
  if ( (PINB & _BV(PCIE_SENSOR)) == 1 && g_currentState == POWER_DOWN) {
    g_currentState = NUDGE_START;
  }
}

void sleep() {
  // We cannot use INT0 as it doesn't work in sleep mode
  GIMSK |= _BV(PCIE); // - Turns on pin change interrupts
  PCMSK |= _BV(PIN_SENSOR); // - Turns on interrupts on pins
  sei();
  ADCSRA &= ~_BV(ADEN); // - Disable ADC, saves ~230uA
  sleep_mode(); // - Go to sleep
  //ADCSRA |= _BV(ADEN); // - Enable ADC
}

void setup() {
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_SENSOR, INPUT);
  //digitalWrite(PIN_SENSOR, HIGH); // Enable pullup resistor

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
}

void loop() {
  switch (g_currentState) {
    case BOOT:
      for(int i = 0; i < 4; i++) {
        digitalWrite(PIN_LED, HIGH);
        delay(200);
        digitalWrite(PIN_LED, LOW);
        delay(200);
      }
      g_currentState = POWER_DOWN;
      break;
    case NUDGE_START:
      g_nudgeStartMillis = millis();
      g_currentState = NUDGE_PROGRESS;
      break;
    case NUDGE_PROGRESS:
#ifdef BLINK_INSTEAD_OF_PULSE
      for(int i = 0; i < 10; i++) {
        digitalWrite(PIN_LED, HIGH);
        delay(200);
        digitalWrite(PIN_LED, LOW);
        delay(200);
      }
#else
      analogWrite(PIN_LED, sin( 2 * PI / NUDGE_PULSE_DURATION * (millis() - g_nudgeStartMillis) + 3 * PI / 2) * 128 + 128);
#endif
      if(millis() - g_nudgeStartMillis > NUDGE_DURATION) {
        g_currentState = NUDGE_STOP;
      }
      break;
    case NUDGE_STOP:
      digitalWrite(PIN_LED, LOW);
      g_currentState = POWER_DOWN;
      break;
    case POWER_DOWN:
      sleep();
      break;
  }
}
