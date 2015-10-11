// ----------------------------------------------------------------------------
// L298N Motor Driver Class
//
// (c) 2014 Dreamshader
// 
// ----------------------------------------------------------------------------

#ifndef _L298N_H_ 
#define _L298N_H_

// ----------------------------------------------------------------------------

#include <stdint.h>
#include <avr/io.h>
// #include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "Arduino.h"

// ----------------------------------------------------------------------------

class L298N
{

public:
  L298N(int In1, int In2, int PWM);

  int getDutyCycle(void);
  int getAbsDutyCycle(void);
  void setDutyCycle(int newCycle);
  void forward(void);
  void backward(void);
  void stopp(void);
  void breakes(void);
  void slowDown(void);
  void speedUp(void);

private:
  int pinIn1;
  int pinIn2;
  int pinPWM;
  int currDutyCycle;
  int absDutyCycle;
  
};

// ----------------------------------------------------------------------------

#endif // _L298N_H_

