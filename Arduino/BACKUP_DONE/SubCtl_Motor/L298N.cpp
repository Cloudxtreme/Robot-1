// ----------------------------------------------------------------------------
// L298N Motor Driver Class
//
// (c) 2014 Dreamshader
// 
// ----------------------------------------------------------------------------

#include "L298N.h"

L298N::L298N(int In1, int In2, int PWM)
{
  pinIn1 = In1;
  pinIn2 = In2;
  pinPWM = PWM;
  currDutyCycle = absDutyCycle = 0;
  pinMode(pinIn1, OUTPUT);
  pinMode(pinIn2, OUTPUT);
  pinMode(pinPWM, OUTPUT);
  stopp();
  analogWrite(pinPWM, currDutyCycle);
}

int L298N::getDutyCycle(void)
{
  return currDutyCycle;
}

int L298N::getAbsDutyCycle(void)
{
  return absDutyCycle;
}

void L298N::setDutyCycle(int newCycle)
{
  currDutyCycle = absDutyCycle = newCycle;
  analogWrite(pinPWM, currDutyCycle);
}

void L298N::slowDown(void)
{
  if( currDutyCycle > 0 )
  {
    currDutyCycle--;
    analogWrite(pinPWM, currDutyCycle);
  }
}

void L298N::speedUp(void)
{
  if( currDutyCycle < 255 )
  {
    currDutyCycle++;
    analogWrite(pinPWM, currDutyCycle);
  }
}

void L298N::forward(void)
{
  digitalWrite(pinIn1, HIGH);
  digitalWrite(pinIn2, LOW);
}

void L298N::backward(void)
{
  digitalWrite(pinIn1, LOW);
  digitalWrite(pinIn2, HIGH);
}

void L298N::stopp(void)
{
  digitalWrite(pinIn1, LOW);
  digitalWrite(pinIn2, LOW);
}


void L298N::breakes(void)
{
  digitalWrite(pinIn1, HIGH);
  digitalWrite(pinIn2, HIGH);
}



