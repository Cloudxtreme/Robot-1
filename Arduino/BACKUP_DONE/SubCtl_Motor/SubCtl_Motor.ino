/*
  Eine einfache Motorsteuerung mit einem Arduino Pro Mini
   
  Es wird die Drehzahl eines Motors/Rades gemessen, das mit einer sog.
  Encoderscheibe versehen ist.
  Der Abgriff erfolgt über eine Gabel-Lichtschranke. Bei einer Umdrehung
  der Scheibe wird der IR-Lichtstrahl 20x unterbrochen.
  Somit ergibt sich als Drehzahl die Anzahl der Unterbrechungen pro Minute
  geteilt durch 20.
  Es wird die Interrupt-Variante für die Interrupts 0 und 1 verwendet.

  Aufbau:
  * zwei TTL Getriebemotoren mit aufgesteckter Encoder-Scheibe
  * zwei Speed Encoder Gabel-Lichtschranken
  * ein L298N Doppel-HBrücken-Board
  * ein Arduino Pro Mini

  (C) 2014 Dirk Schanz aka Dreamshader
  This example code is in the public domain.
 
 */
 
#include "HC020K.h"
#include "L298N.h"
#include "SyncMotor.h"
 
#include "MsTimer2.h"
#include <Wire.h>
 
 
//
// !!!! Interrupt-Pin for Signalling RPi is D10 !!!!
//
 
 
 
// we'll see what to do with this 
const int ledPin = 13;       // the pin that the LED is attached to

// we have two speed encoders - one for the left, another for 
// the right wheel - we use interrupts int0 and int1 on 
// digital pins 2 and 3
//
const int rightEncoderPin = 2;
const int leftEncoderPin = 3;

// right motor
// Out1 and Out2
// are controlled by
// pinIn1 and pinIn2
const int pinIn1 = 9;
const int pinIn2 = 8;

// left motor
// Out3 and Out4 -> 
// are controlled by
// pinIn3 and pinIn4
const int pinIn3 = 7;
const int pinIn4 = 6;

// control pins to LN298 board
// ENA and ENB are used for speed control
// use PWM on pins 4 and 5
const int pinRightPWM = 5;
const int pinLeftPWM = 4;

const int MaxRightDuty = 255;
const int MaxLeftDuty = 255;

const int MinRightDuty = 60;
const int MinLeftDuty = 60;

const int StopRightDuty = 0;
const int StopLeftDuty = 0;

volatile int PWMLeftDutyCycle;
volatile int PWMRightDutyCycle;

volatile int CurrDirection;

const int dirForward = 1;
const int dirBackward = 2;
const int dirStopp = 3;
const int dirBreak = 4;

const int rightEncIrq = 0;
const int leftEncIrq = 1;

#define ENC_PULSE_PER_REVOLUTION  20

HC020K *rightEncoder;
HC020K *leftEncoder;
L298N *rightMotor;
L298N *leftMotor;
SyncMotor *pEngine;

#define ON  LOW
#define OFF  HIGH

#define MOTOR_CMD_BUFSIZ  50

char commandString[MOTOR_CMD_BUFSIZ];
#define SERIAL_BAUD 57600

#define I2C_SLAVE_MOTOR  0x0a

void rightISR(void)
{
  rightEncoder->pCallback();
}

void leftISR(void)
{
  leftEncoder->pCallback();
}

void timerIsr(void) {
  pEngine->syncService();
}

void i2cReceive(int howMany)
{
  while(1 < Wire.available()) // loop through all but the last
  {
    char c = Wire.read(); // receive byte as a character
    Serial.print(c);         // print the character
  }
  int x = Wire.read();    // receive byte as an integer
  Serial.println(x);         // print the integer
}

void setup() {

  // initialize serial monitor
  Serial.begin(SERIAL_BAUD);

  Wire.begin(I2C_SLAVE_MOTOR); // join i2c bus
  Wire.onReceive(i2cReceive); // register event
    
  rightEncoder = new HC020K(rightEncoderPin, rightEncIrq, FALLING, 
      ENC_PULSE_PER_REVOLUTION);
  rightMotor = new L298N( pinIn1, pinIn2, pinRightPWM );
  rightEncoder->attachISR(rightISR);

  leftEncoder = new HC020K(leftEncoderPin, leftEncIrq, FALLING,
      ENC_PULSE_PER_REVOLUTION);
  leftMotor = new L298N( pinIn3, pinIn4, pinLeftPWM );
  leftEncoder->attachISR(leftISR);

  pEngine = new SyncMotor(rightMotor, rightEncoder, leftMotor, leftEncoder);
  pEngine->attachService(2, timerIsr);
  
  // initialize the LED as an output:
  pinMode(ledPin, OUTPUT);

  CurrDirection = dirStopp;

  pEngine->setDutyCycle(254);
}

void toggleLED()
{
  digitalWrite(ledPin, ON);
  delay(200);
  digitalWrite(ledPin, OFF);
  delay(200);
}

void signalLED(int howmany)
{
  int i;
  
  for(i=0; i < howmany; i++)
  {
    toggleLED();
  }
}

void toggleDirection()
{
        if( CurrDirection == dirBreak )
        {
          CurrDirection = dirForward;
          Serial.println("... forward");
          signalLED(2);
          pEngine->forward();

        }
        else
        {

          if( CurrDirection == dirForward)
          {
            CurrDirection = dirStopp;
            Serial.println("... stopping");
            signalLED(3);
            pEngine->stopp();

          }
          else
          {
            if( CurrDirection == dirStopp )
            {
              CurrDirection = dirBackward;
              Serial.println("... backward");
              signalLED(4);
              pEngine->backward();
            }
            else
            {
              CurrDirection = dirBreak;
              Serial.println("... breakes");
              signalLED(5);
              pEngine->breakes();
            }
          }
        }
}

void onboardLED(int state)
{
  digitalWrite(ledPin, state);
}

int parseCmd( char* cmdString, int* pForw, int* pBack, int* pRight, int* pLeft)
{
  int rc = -1;

  if( cmdString != NULL )
  {
    rc = sscanf(cmdString, "%d:%d:%d:%d", pForw, pBack, pRight, pLeft);
  }

  return(rc);

}

void loop() {
  static unsigned long currTime;
  static unsigned long lastTime;
  static int inIdx;
  int Forw, Back, Right, Left;
  int cmdArgs;


  if( lastTime != 0 )
  {

    currTime = millis();
    // Serial.print("millis diff: ");
    // Serial.println(currTime - lastTime);
    // Serial.print(".");

    if( ((currTime - lastTime) / 1000) % 10 == 0 )
    {            
      Serial.println("-> go! ");
      toggleDirection();
      Serial.print( rightEncoder->getCurrPulses() );
      Serial.print(" (");
      Serial.print( leftEncoder->getCurrPulses() );
      Serial.println(")");
    }

  }
  else
  {
    lastTime = millis();
  }
  
  while (Serial.available()) 
  {
    char inChar = (char)Serial.read(); 

    commandString[inIdx++] = inChar;
    commandString[inIdx] = '\0';

    if (inIdx == 15 || inChar == '\n') {
      Serial.println(commandString);
      cmdArgs = parseCmd( commandString, &Forw, &Back, &Right, &Left );
      Serial.print("CMD: ");
      Serial.print(cmdArgs);
      Serial.print(" -> ");
      Serial.print(Forw);
      Serial.print(", ");
      Serial.print(Back);
      Serial.print(", ");
      Serial.print(Right);
      Serial.print(", ");
      Serial.println(Left);
      inIdx = 0;
      commandString[inIdx] = '\0';      
    } 
  }  
  
  onboardLED(ON);
  delay(1000);
  onboardLED(OFF);  
}


