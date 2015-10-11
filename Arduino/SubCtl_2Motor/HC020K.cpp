// ----------------------------------------------------------------------------
// L298N Motor Driver Class
//
// (c) 2014 Dreamshader
// 
// ----------------------------------------------------------------------------

#include "HC020K.h"


HC020K::HC020K(int Pin, int Irq, int Trigger, int PPR)
{
  pinIrq = Pin;
  numIrq = Irq;
  encPPR = PPR; // Pulses Per Revolution
  triggerIrq = Trigger;
  pulsCycle = DEFAULT_CYCLE;
  currPulses = 0;
  prevPulses = 0;
  irqNoService = 1;
  
  // initialize the encoder pin as input
  pinMode(pinIrq, INPUT);
  digitalWrite(pinIrq, HIGH);  

}

void HC020K::attachISR( void (*isr)(void) )
{
  attachInterrupt(numIrq, isr, triggerIrq);
  irqNoService = 0;
}

int HC020K::getPPR(void)
{
  return encPPR;
}

int HC020K::getPulseCycle(void)
{
  return pulsCycle;
}

long HC020K::getCurrPulses(void)
{
  return currPulses;
}

void HC020K::setPPR(int PPR)
{
  encPPR = PPR;
}

void HC020K::setPulseCycle(int pCycles)
{
  pulsCycle = pCycles;
}

void HC020K::reset(void)
{
  irqNoService = 1;
  
  currPulses = 0;
  prevPulses = 0;
  
  irqNoService = 0;
}

void HC020K::pCallback(void)
{
  if( irqNoService )
    return;
    
  currPulses++;
}


