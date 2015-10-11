// ----------------------------------------------------------------------------
// HC020K Encoder Disc Driver Class
//
// (c) 2014 Dreamshader
// 
// ----------------------------------------------------------------------------

#ifndef _HC020K_H_ 
#define _HC020K_H_

// ----------------------------------------------------------------------------

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "Arduino.h"

// ----------------------------------------------------------------------------

#define DEFAULT_CYCLE 20
#define DEFAULT_TRIGGER RISING

class HC020K
{

public:
  HC020K(int Pin, int Irq, int Trigger, int PPR);

public:
  int getPPR(void);
  int getPulseCycle(void);
  long getCurrPulses(void);

  void setPPR(int PPR);
  void setPulseCycle(int pCycles);
  void reset(void);
  void pCallback(void);
  void attachISR( void (*isr)(void) );

public:
  volatile uint8_t irqNoService;
  
private:
  int pinIrq;
  int numIrq;
  int triggerIrq;
  int encPPR; // Pulses Per Revolution
  int pulsCycle;
  long currPulses;
  long prevPulses;
  long deltaPulses;
};

// ----------------------------------------------------------------------------

#endif // _HC020K_H_

