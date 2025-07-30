#include "PWM.h"

pwm::pwm(void) {
}
 
pwm::~pwm() { 
}

void pwm::begin(){
  setPWM(0);
  TCCR1A = 0b10000000;
  TCCR1B = 0b00010001;   
  ICR1 = MAXFORCE;
  OCR1A = 0;
}
 
void pwm::setPWM(int16_t force) {

 	force = constrain(abs(force),0,250);
	int16_t normalizedForce;

	if (force > 5) {
		normalizedForce = map(force,0,250, 150,MAXFORCE);
	} else {
		normalizedForce = 0;
	}

	PWM9 = normalizedForce;
}
 
