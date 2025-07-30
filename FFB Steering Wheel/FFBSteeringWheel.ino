#include "Joystick.h" //initialize joystick library for ffb and game input

#include "DigitalWriteFast.h" //initialize digital read and write library for readig encoder and buttons

//inbuilt analogWrite have low frequency so use timer to make higher frequency
#include "PWM.h" // initialize pwm library for motor
pwm motorPWM; // create an object for class pwm

const int8_t steeringDeadZone = 20;

// arduion clu-A0 brkA1 cluA2 hndA3 shXA4 shYA5 0 1 encA2 encB3 4 5 6 motA7 motB8 motPWM9 seq+10 seq-11 12 13

// define encoder pin
#define encoderPinA 2
#define encoderPinB 3

// define motor pin and pwm can only use 9 10 and 11 for timer pwm also only pin 9 is accesable with this pwm library
#define motorPinA 7
#define motorPinB 8
#define motorPinPWM 9

// define analog input
#define clutchPin A0
#define breakPin A1
#define acceleratorPin A2
#define handbreakPin A3

// shifter pin 
#define shifterPinA A4
#define shifterPinB A5

//encoder min and max value will add to eeprom in future
#define ENCODER_MAX_VALUE 3340
#define ENCODER_MIN_VALUE -3340

//the pwm value for motor if motor go beyond min or max encoder value
#define MAX_PWM 200
//flag to check if encoder out of min or max
bool isOutOfRange = false;

//initialize force for X axis (dir is 90 for left and 270 for right)
int32_t forces[2]={0};

//initialize gain
Gains gains[1];

//initialize joystick inputes 
Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID,JOYSTICK_TYPE_JOYSTICK,
  12, 4,                  // Button Count, Hat Switch Count
  true, true, true,     // X, Y, Z Axis || Steering, Accelerator, Hand Break
  true, true, false,   //  Rx, Ry, no Rz          || Clutch, Break
  false, false,          // No rudder or throttle
  false, false, false);    // No accelerator, brake, or steering

//Encoder section
volatile int32_t  rawEncoderPosition  = 0;
volatile int8_t oldState = 0;
const int8_t KNOBDIR[] = {
  0, 1, -1, 0,
  -1, 0, 0, 1,
  1, 0, 0, -1,
  0, -1, 1, 0
};
void tick(void)
{
  uint8_t sig1 = digitalReadFast(encoderPinA);
  uint8_t sig2 = digitalReadFast(encoderPinB);
  int8_t thisState = sig1 | (sig2 << 1);

  if (oldState != thisState) {
    rawEncoderPosition += KNOBDIR[thisState | (oldState<<2)];
    oldState = thisState;
  }
}

void setup() {
	Serial.begin(115200);

	gains[0].totalGain         = 80;
	gains[0].constantGain      = 70;

  pinMode(motorPinA, OUTPUT);
  pinMode(motorPinB, OUTPUT);
  pinMode(motorPinPWM, OUTPUT);

  pinMode(clutchPin, INPUT);
  pinMode(breakPin, INPUT);
  pinMode(acceleratorPin, INPUT);
  pinMode(handbreakPin, INPUT);

  pinMode(shifterPinA, INPUT);
  pinMode(shifterPinB, INPUT);

  pinMode(encoderPinA, INPUT_PULLUP);
  pinMode(encoderPinB, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(encoderPinA),tick,CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoderPinB),tick,CHANGE);

  Joystick.setXAxisRange(ENCODER_MIN_VALUE, ENCODER_MAX_VALUE);
  Joystick.setYAxisRange(0, 500);
  Joystick.setZAxisRange(0, 500);
  Joystick.setRxAxisRange(0, 500);
  Joystick.setRyAxisRange(0, 500);
  
 

  Joystick.setGains(gains);
  Joystick.begin(true);

  motorPWM.begin();
  motorPWM.setPWM(0);
  
  cli();
  TCCR3A = 0; //set TCCR1A 0
  TCCR3B = 0; //set TCCR1B 0
  TCNT3  = 0; //counter init
  OCR3A = 399;
  TCCR3B |= (1 << WGM32); //open CTC mode
  TCCR3B |= (1 << CS31); //set CS11 1(8-fold Prescaler)
  TIMSK3 |= (1 << OCIE3A);
  sei();
  
}

ISR(TIMER3_COMPA_vect) {
  Joystick.getUSBPID();
}

void loop() {

  int16_t currentPosition = rawEncoderPosition;

  if (abs(currentPosition) < steeringDeadZone) currentPosition = 0;

  
  if(currentPosition > ENCODER_MAX_VALUE)
  {
    isOutOfRange = true;
    currentPosition = ENCODER_MAX_VALUE;
  }else if(currentPosition < ENCODER_MIN_VALUE)
  {
    isOutOfRange = true;
    currentPosition = ENCODER_MIN_VALUE;
  }else{
    isOutOfRange = false;
  }

  Joystick.setXAxis(currentPosition); //Steering
  Joystick.setYAxis(0);               //Accelerator
  Joystick.setZAxis(0);               //Hand Break
  Joystick.setRxAxis(0);              //Clutch
  Joystick.setRyAxis(0);              //Break
  
  Joystick.getForce(forces);
  
  if(!isOutOfRange){
    if(forces[0] > 0)
    {
      digitalWrite(motorPinA, HIGH);
      digitalWrite(motorPinB, LOW);
    }else if(forces[0] < 0){
      digitalWrite(motorPinA, LOW);
      digitalWrite(motorPinB, HIGH);
    }else{
      digitalWrite(motorPinA, LOW);
      digitalWrite(motorPinB, LOW);
    }
    motorPWM.setPWM(forces[0]);
  }else{
    if(currentPosition > 0){
      digitalWrite(motorPinA, HIGH);
      digitalWrite(motorPinB, LOW);
    }else{
      digitalWrite(motorPinA, LOW);
      digitalWrite(motorPinB, HIGH);
    }
    motorPWM.setPWM(MAX_PWM);
  }

  //Serial.print(" ");
  //Serial.print(currentPosition);
  //Serial.print(" ");

}
