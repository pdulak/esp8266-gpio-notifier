#include "Arduino.h"
#include "BlinkLed.h"

BlinkLed::BlinkLed( int interval, int led_pin ) {
  _led_state = LOW;
  _led_pin = led_pin;
  _interval = interval;
  pinMode( _led_pin, OUTPUT );
}

void BlinkLed::Blink( void ) {
  unsigned long current_time = millis();

  if ( (_when_changed + _interval) < current_time ) {
    _when_changed = current_time;
    if (_led_state == LOW) _led_state = HIGH; else _led_state = LOW;
    digitalWrite( _led_pin, _led_state );
  }
}

void BlinkLed::UpdateBlink( int interval, int state ) {
  _led_state = state;
  _interval = interval;
}
