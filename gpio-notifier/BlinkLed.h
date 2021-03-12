#ifndef BLINKLED_H_INCLUDED
#define BLINKLED_H_INCLUDED

class BlinkLed
{
  
  private:
    int _interval;
    int _led_pin;
    int _led_state;
    unsigned long _when_changed;
    
  public:
    BlinkLed(int interval, int led_pin);
    void Blink();
    void UpdateBlink(int interval, int state);
    
};

#endif
