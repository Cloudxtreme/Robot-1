// ----------------------------------------------------------------------------
// HC020K Encoder Disc Driver Class
//
// (c) 2014 Dreamshader
// 
// ----------------------------------------------------------------------------

#ifndef _SYNCMOTOR_H_ 
#define _SYNCMOTOR_H_

// ----------------------------------------------------------------------------

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "Arduino.h"


#include "HC020K.h"
#include "L298N.h"

#define MIN_DUTY_LIMIT  15
#define MAX_DUTY_LIMIT  15

// ----------------------------------------------------------------------------

class SyncMotor
{

public:

  typedef enum MotStat_e {
    offline = 0, asynchron, syncing, synchron, locked } MotorStatus;
   
  SyncMotor(L298N *rightMotor,HC020K *rightEncoder,L298N *leftMotor,HC020K *leftEncoder);
  void attachService( long serviceHertz, void (*isr)(void) );
  void syncService(void);
  
public:
  MotorStatus getStatus(void);
  void begin(void);
  void end(void);
  void stopEncoderService(void);
  void startEncoderService(void);

  void getDutyCycle(int *pRightCycle, int *pLeftCycle);
  void setDutyCycle(int newCycle);
  void forward(void);
  void backward(void);
  void stopp(void);
  void breakes(void);
  void reset(void);
  
  
  void setLimits( int newLowerLimit, int newUpperLimit);
  int getLowerLimit(void);
  int getUpperLimit(void);  


private:
  long delta;
  MotorStatus SyncStatus;
  int lowerLimit;
  int upperLimit;
  L298N *synRightMotor;
  HC020K *synRightEncoder;
  L298N *synLeftMotor;
  HC020K *synLeftEncoder;
  

};

// ----------------------------------------------------------------------------

#endif // _SYNCMOTOR_H_

