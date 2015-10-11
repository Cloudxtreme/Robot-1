// ----------------------------------------------------------------------------
// L298N Motor Driver Class
//
// (c) 2014 Dreamshader
// 
// ----------------------------------------------------------------------------

#include "SyncMotor.h"
#include "MsTimer2.h"
// #include "TimerOne.h"


SyncMotor::SyncMotor(L298N *rightMotor,HC020K *rightEncoder,L298N *leftMotor,HC020K *leftEncoder)
{
  
  synRightMotor = rightMotor;
  synRightEncoder = rightEncoder;
  synLeftMotor = leftMotor;
  synLeftEncoder = leftEncoder;

  lowerLimit = 0;
  upperLimit = 0;  
  SyncStatus = offline;  
  delta = 0;
}



void SyncMotor::attachService( long serviceHertz, void (*isr)(void) )
{
//  Timer1.initialize(1000);
//  Timer1.attachInterrupt(isr);
  MsTimer2::set(500, isr); // 500ms period  
  MsTimer2::start();
}

void SyncMotor::stopEncoderService(void)
{
  synRightEncoder->irqNoService = synLeftEncoder->irqNoService = 1;
}

void SyncMotor::startEncoderService(void)
{
  synRightEncoder->irqNoService = synLeftEncoder->irqNoService = 0;
}

void SyncMotor::setDutyCycle(int newCycle)
{
  synRightMotor->setDutyCycle(newCycle);
  synLeftMotor->setDutyCycle(newCycle);
}

void SyncMotor::getDutyCycle(int *pRightCycle, int *pLeftCycle)
{
  *pRightCycle = synRightMotor->getDutyCycle();
  *pLeftCycle = synLeftMotor->getDutyCycle();  
}

void SyncMotor::setLimits( int newLowerLimit, int newUpperLimit)
{
  lowerLimit = newLowerLimit;
  upperLimit = newUpperLimit;
}

void SyncMotor::reset(void)
{
  synRightEncoder->reset();
  synLeftEncoder->reset();
}

int SyncMotor::getLowerLimit(void)
{
  return lowerLimit;
}

int SyncMotor::getUpperLimit(void)  
{
  return upperLimit;
}

void SyncMotor::forward(void)
{
  stopEncoderService();
  synRightMotor->forward();
  synLeftMotor->forward();
  startEncoderService();  
}

void SyncMotor::backward(void)
{
  stopEncoderService();
  synRightMotor->backward();
  synLeftMotor->backward();
  startEncoderService();
}

void SyncMotor::stopp(void)
{
  stopEncoderService();
  synRightMotor->stopp();
  synLeftMotor->stopp();
}

void SyncMotor::breakes(void)
{
  stopEncoderService();
  synRightMotor->breakes();
  synLeftMotor->breakes();
}

SyncMotor::MotorStatus SyncMotor::getStatus(void)
{
  return SyncStatus;
}

void SyncMotor::begin(void)
{
  SyncStatus = syncing;
}

void SyncMotor::end(void)
{
  SyncStatus = offline;
}

#define UPPER_TICK_RANGE  5
#define LOWER_TICK_RANGE -5

void SyncMotor::syncService(void)
{
  // here we go for syncing ...
  int dutyCycle;
  stopEncoderService();

  delta = synRightEncoder->getCurrPulses() - synLeftEncoder->getCurrPulses();

  if( delta > UPPER_TICK_RANGE )
  {
    // right motor is too fast ... speed up left or slow down right motor
    Serial.println("Right too fast");
    // speed up left motor possible?
    dutyCycle = synLeftMotor->getDutyCycle();
    if( dutyCycle < 254 && dutyCycle < (synLeftMotor->getAbsDutyCycle() + MAX_DUTY_LIMIT) )
    {
      Serial.print("speed up left motor ");
      Serial.println(dutyCycle);
      synLeftMotor->speedUp();
      reset();
    }
    else
    {
      dutyCycle = synRightMotor->getDutyCycle();
      if( dutyCycle > (synRightMotor->getAbsDutyCycle() - MIN_DUTY_LIMIT) )
      {
        Serial.print("slow down right motor ");
        Serial.println(dutyCycle);
        synRightMotor->slowDown();
        reset();
      }
      else
      {
        // grübel ...
        Serial.print("right ");
        Serial.print( synRightEncoder->getCurrPulses() );
        Serial.print(" -> left ");
        Serial.println( synLeftEncoder->getCurrPulses() );
        reset();
      }
    }
  }
  else
  {
    if( delta < LOWER_TICK_RANGE )
    {
      // left motor is too fast ... speed up right or slow down left motor
      Serial.println("Left too fast");
      // speed up right motor possible?
      dutyCycle = synRightMotor->getDutyCycle();
      if( dutyCycle < 254 && dutyCycle < (synRightMotor->getAbsDutyCycle() + MAX_DUTY_LIMIT) )
      {
        Serial.print("speed up right motor ");
        Serial.println(dutyCycle);
        synRightMotor->speedUp();
        reset();
      }
      else
      {
        dutyCycle = synLeftMotor->getDutyCycle();
        if( dutyCycle > (synLeftMotor->getAbsDutyCycle() - MIN_DUTY_LIMIT) )
        {
          Serial.print("slow down left motor ");
          Serial.println(dutyCycle);
          synLeftMotor->slowDown();
          reset();
        }
        else 
        {
          // grübel ...
          Serial.print("right ");
          Serial.print( synRightEncoder->getCurrPulses() );
          Serial.print(" -> left ");
          Serial.println( synLeftEncoder->getCurrPulses() );
          reset();
        }
      }
    }
  }
  
// MAX_DUTY_LIMIT  
// MIN_DUTY_LIMIT
// synRightMotor->getDutyCycle
// synLeftMotor->getDutyCycle
// synRightMotor->getAbsDutyCycle
// synLeftMotor->getAbsDutyCycle
// synRightEncoder->getCurrPulses
// synLeftEncoder->getCurrPulses
// synRightMotor->slowDown
// synLeftMotor->slowDown
// synRightMotor->speedUp
// synLeftMotor->speedUp  

  startEncoderService();
 
}
