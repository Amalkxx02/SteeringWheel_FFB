#ifndef PWM_H
#define PWM_H
#include <Arduino.h>

#define PWM9   OCR1A
#define PWM_FREQ 20000.0f
#define MAXFORCE (F_CPU/(PWM_FREQ*2)) //16000000 is system clock of Leonardo

class pwm {
  public:   
  pwm(void);
  ~pwm(void);
  void begin();
  void setPWM(int16_t force);
};

#endif
