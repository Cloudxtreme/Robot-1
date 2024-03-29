/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the Uno and
  Leonardo, it is attached to digital pin 13. If you're unsure what
  pin the on-board LED is connected to on your Arduino model, check
  the documentation at http://arduino.cc

  This example code is in the public domain.

  modified 8 May 2014
  by Scott Fitzgerald
 */

#include <TimerOne.h>
#include <MsTimer2.h>
#include <Wire.h>
#include <EEPROM.h>

#include "RobotI2C_defs.h"
#include "RobotI2C_telegrams.h"


#define PIN_RIGHT_HIGH_FRONT  12
#define PIN_RIGHT_TURN_FRONT  13
#define PIN_LEFT_LOW_FRONT    11

#define PIN_LEFT_HIGH_FRONT   9
#define PIN_LEFT_TURN_FRONT  10
#define PIN_RIGHT_LOW_FRONT   8

#define PIN_LEFT_BACK         5
#define PIN_LEFT_REAR         7
#define PIN_LEFT_BRAKE_REAR   7
#define PIN_LEFT_TURN_REAR    6

#define PIN_RIGHT_BACK        2
#define PIN_RIGHT_REAR        4
#define PIN_RIGHT_BRAKE_REAR  4
#define PIN_RIGHT_TURN_REAR   3

#define PIN_ROTATION_1       A0
#define PIN_ROTATION_2       A1
#define PIN_ROTATION_3       A2
#define PIN_ROTATION_4       A3

#define ROTATION_INTVAL     50
#define ROTATION_FLOAT_TRANS          5

#define ON   1
#define OFF  0

#define LIGHT_OFF   0
#define LIGHT_ON    2
#define LIGHT_HIGH  4
#define LIGHT_REAR  8

#define CLOCKWISE    1
#define CCLOCKWISE   2

volatile int ledFlashTurn_Front = 0;
volatile int ledFlashTurn_Rear = 0;

volatile int ledBrake_Left = 0;
volatile int ledBrake_Right = 0;

volatile int ledRotationLight;
volatile int ledRotationDir = 0;
volatile int ledRotationState = 0;

volatile int lighState;


volatile char i2cInBuf[I2C_BUF_SIZE];
char i2cOutBuf[I2C_BUF_SIZE];
volatile int i2cInIdx;

int i2cSlaveAddr;


//
// ------------------ INTERRUPT HANDLING -------------------
//
void timerTurn_Isr()
{
  if( ledFlashTurn_Front != 0 )
  {
    digitalWrite(ledFlashTurn_Front, digitalRead(ledFlashTurn_Front) ^1 );
  }
  if( ledFlashTurn_Rear != 0 )
  {
    digitalWrite(ledFlashTurn_Rear, digitalRead(ledFlashTurn_Rear) ^1 );
  }
}

void timerBrakes_Isr()
{

  if( ledBrake_Left != 0 )
  {
    digitalWrite(ledBrake_Left, digitalRead(ledBrake_Left) ^1 );
  }
  if( ledBrake_Right != 0 )
  {
    digitalWrite(ledBrake_Right, digitalRead(ledBrake_Right) ^1 );
  }
}


// ---------------------------------------------------------
// receive all iic data
// ---------------------------------------------------------

void i2cReceive(int howMany)
{
  while( Wire.available() ) 
  {
    char c = Wire.read(); 
    if( i2cInIdx < (I2C_BUF_SIZE - 1) )
    {
      i2cInBuf[i2cInIdx++] = c;
    }
  }
  i2cInBuf[i2cInIdx] = '\0';
  i2cInIdx = 0;
}

  
// ---------------------------------------------------------
// send response to calling part
// ---------------------------------------------------------
void i2cResponse( char* msg )
{
  sprintf(i2cOutBuf, "%s", msg);
  Wire.write((uint8_t*) i2cOutBuf, strlen(i2cOutBuf)+1);   
}


// ---------------------------------------------------------
// i2cRequest callback - triggered on incoming request
// ---------------------------------------------------------
void i2cRequest(void)
{
  char ctoken;
  char xtoken;
  static char responseBuffer[I2C_BUF_SIZE];
  
  if( strlen((char*) i2cInBuf) >= 2 )
  {
    if( i2cInBuf[0] == '%' )
    {
      Serial.println( (char*) i2cInBuf );
      ctoken = i2cInBuf[1];
      // i2cResponse((char*) responseBuffer);
    }  
    else
    {
      i2cResponse( "%?%err" );
    }
  }
  else
  {
    i2cResponse( "%?%err" );
  }    
}



void doRotationLight()
{
//
  static long lastMillis = 0;

  if( ledRotationState == ON )
  {
    if( (millis() - lastMillis ) >= ROTATION_INTVAL )
    {
      lastMillis = millis();
      switch(ledRotationLight)
      {
        case PIN_ROTATION_1:
          if( ledRotationDir == CLOCKWISE )
          {
            digitalWrite(PIN_ROTATION_2, digitalRead(ledRotationLight) );
            delay(ROTATION_FLOAT_TRANS);
            digitalWrite(ledRotationLight, digitalRead(ledRotationLight) ^ 1);
            ledRotationLight = PIN_ROTATION_2;
          }
          else
          {
            if( ledRotationDir == CCLOCKWISE )
            {
              digitalWrite(PIN_ROTATION_4, digitalRead(ledRotationLight) );
              delay(ROTATION_FLOAT_TRANS);
              digitalWrite(ledRotationLight, digitalRead(ledRotationLight) ^ 1);
              ledRotationLight = PIN_ROTATION_4;
            }
          }
          break;
        case PIN_ROTATION_2:
          if( ledRotationDir == CLOCKWISE )
          {
            digitalWrite(PIN_ROTATION_3, digitalRead(ledRotationLight) );
            delay(ROTATION_FLOAT_TRANS);
            digitalWrite(ledRotationLight, digitalRead(ledRotationLight) ^ 1);
            ledRotationLight = PIN_ROTATION_3;
          }
          else
          {
            if( ledRotationDir == CCLOCKWISE )
            {
              digitalWrite(PIN_ROTATION_1, digitalRead(ledRotationLight) );
              delay(ROTATION_FLOAT_TRANS);
              digitalWrite(ledRotationLight, digitalRead(ledRotationLight) ^ 1);
              ledRotationLight = PIN_ROTATION_1;
            }
          }
          break;
        case PIN_ROTATION_3:
          if( ledRotationDir == CLOCKWISE )
          {
            digitalWrite(PIN_ROTATION_4, digitalRead(ledRotationLight) );
            delay(ROTATION_FLOAT_TRANS);
            digitalWrite(ledRotationLight, digitalRead(ledRotationLight) ^ 1);
            ledRotationLight = PIN_ROTATION_4;
          }
          else
          {
            if( ledRotationDir == CCLOCKWISE )
            {
              digitalWrite(PIN_ROTATION_2, digitalRead(ledRotationLight) );
              delay(ROTATION_FLOAT_TRANS);
              digitalWrite(ledRotationLight, digitalRead(ledRotationLight) ^ 1);
              ledRotationLight = PIN_ROTATION_2;
            }
          }
          break;
        case PIN_ROTATION_4:
          if( ledRotationDir == CLOCKWISE )
          {
            digitalWrite(PIN_ROTATION_1, digitalRead(ledRotationLight) );
            delay(ROTATION_FLOAT_TRANS);
            digitalWrite(ledRotationLight, digitalRead(ledRotationLight) ^ 1);
            ledRotationLight = PIN_ROTATION_1;
          }
          else
          {
            if( ledRotationDir == CCLOCKWISE )
            {
              digitalWrite(PIN_ROTATION_3, digitalRead(ledRotationLight) );
              delay(ROTATION_FLOAT_TRANS);
              digitalWrite(ledRotationLight, digitalRead(ledRotationLight) ^ 1);
              ledRotationLight = PIN_ROTATION_3;
            }
          }
          break;
        default:
          break;
      } // switch(ledRotationLight)
    } // if( (millis() - LastMillis ) >= ROTATION_INTVAL )
  } // if( ledRotationState == ON )
}
//
// ---------------- END INTERRUPT HANDLING -----------------
//


void setup() 
{
  Serial.begin(57600);

  pinMode(PIN_RIGHT_LOW_FRONT, OUTPUT);
  pinMode(PIN_RIGHT_HIGH_FRONT, OUTPUT);
  pinMode(PIN_RIGHT_TURN_FRONT, OUTPUT);

  pinMode(PIN_LEFT_LOW_FRONT, OUTPUT);
  pinMode(PIN_LEFT_HIGH_FRONT, OUTPUT);
  pinMode(PIN_LEFT_TURN_FRONT, OUTPUT);

  digitalWrite(PIN_RIGHT_LOW_FRONT, LOW);
  digitalWrite(PIN_RIGHT_HIGH_FRONT, LOW);
  digitalWrite(PIN_RIGHT_TURN_FRONT, LOW);

  digitalWrite(PIN_LEFT_LOW_FRONT, LOW);
  digitalWrite(PIN_LEFT_HIGH_FRONT, LOW);
  digitalWrite(PIN_LEFT_TURN_FRONT, LOW);

  pinMode(PIN_LEFT_REAR, OUTPUT);
  pinMode(PIN_LEFT_BRAKE_REAR, OUTPUT);
  pinMode(PIN_LEFT_TURN_REAR, OUTPUT);

  pinMode(PIN_RIGHT_REAR, OUTPUT);
  pinMode(PIN_RIGHT_BRAKE_REAR, OUTPUT);
  pinMode(PIN_RIGHT_TURN_REAR, OUTPUT);

  digitalWrite(PIN_LEFT_REAR, LOW);
  digitalWrite(PIN_LEFT_BRAKE_REAR, LOW);
  digitalWrite(PIN_LEFT_TURN_REAR, LOW);

  digitalWrite(PIN_RIGHT_REAR, LOW);
  digitalWrite(PIN_RIGHT_BRAKE_REAR, LOW);
  digitalWrite(PIN_RIGHT_TURN_REAR, LOW);

  pinMode(PIN_ROTATION_1, OUTPUT);
  pinMode(PIN_ROTATION_2, OUTPUT);
  pinMode(PIN_ROTATION_3, OUTPUT);
  pinMode(PIN_ROTATION_4, OUTPUT);

  digitalWrite(PIN_ROTATION_1, LOW);
  digitalWrite(PIN_ROTATION_2, LOW);
  digitalWrite(PIN_ROTATION_3, LOW);
  digitalWrite(PIN_ROTATION_4, LOW);

  ledFlashTurn_Front = OFF;
  ledFlashTurn_Rear = OFF;

  ledBrake_Left = 0;
  ledBrake_Right = 0;

  ledRotationDir = OFF;
  ledRotationState = OFF;

  lighState = LIGHT_OFF;

  Timer1.initialize(500000);
  Timer1.stop();

  MsTimer2::set(50, timerBrakes_Isr);
  MsTimer2::stop();

  // setup IIC communication
  Wire.begin(i2cSlaveAddr); // join i2c bus
  Wire.onReceive(i2cReceive); // register event  
  Wire.onRequest(i2cRequest); // register request

}


void flashTurn( int pin1, int pin2, int state )
{
  if( state == ON )
  {
    if( (ledFlashTurn_Front = pin1) != 0 )
    {
      digitalWrite(ledFlashTurn_Front, HIGH);
    }
    if( (ledFlashTurn_Rear = pin2) != 0 )
    {
      digitalWrite(ledFlashTurn_Rear, HIGH);
    }
    Timer1.start();
    Timer1.attachInterrupt(timerTurn_Isr);
  }
  else
  {
    Timer1.detachInterrupt();
    Timer1.stop();
    if( ledFlashTurn_Front != 0 )
    {
      digitalWrite(ledFlashTurn_Front, LOW);
      ledFlashTurn_Front = 0;
    }
    if( ledFlashTurn_Rear != 0 )
    {
      digitalWrite(ledFlashTurn_Rear, LOW);
      ledFlashTurn_Rear = 0;
    }
  }
}

void flashBreakLights( int pinRight, int pinLeft, int state )
{
  if( state == ON )
  {
    if( (ledBrake_Left = pinLeft) != 0 )
    {
      digitalWrite(ledBrake_Left, HIGH);
    }
    if( (ledBrake_Right = pinRight) != 0 )
    {
      digitalWrite(ledBrake_Right, HIGH);
    }
    MsTimer2::start();
  }
  else
  {
    MsTimer2::stop();
    if( (ledBrake_Left = pinLeft) != 0 )
    {
      if( lighState == LIGHT_OFF )
      {
        digitalWrite(ledBrake_Left, LOW);
      }
      else
      {
        digitalWrite(ledBrake_Left, HIGH);
      }
    }
    if( (ledBrake_Right = pinRight) != 0 )
    {
      if( lighState == LIGHT_OFF )
      {
        digitalWrite(ledBrake_Right, LOW);
      }
      else
      {
        digitalWrite(ledBrake_Left, HIGH);
      }
    }
  }
}




void switchRotationLight( int direction, int state )
{  
  if( (ledRotationState = state) == ON )
  {
    digitalWrite(PIN_ROTATION_1, HIGH);
    digitalWrite(PIN_ROTATION_2, LOW);
    digitalWrite(PIN_ROTATION_3, LOW);
    digitalWrite(PIN_ROTATION_4, LOW);
    ledRotationLight = PIN_ROTATION_1;
    ledRotationDir = direction;
  }
  else
  {
    digitalWrite(PIN_ROTATION_1, LOW);
    digitalWrite(PIN_ROTATION_2, LOW);
    digitalWrite(PIN_ROTATION_3, LOW);
    digitalWrite(PIN_ROTATION_4, LOW);
    ledRotationLight = 0;
    ledRotationDir = OFF;
  }
}



void LightsOn(void)
{
  if( lighState == LIGHT_OFF )
  {
Serial.println("Lights on");
    digitalWrite(PIN_RIGHT_LOW_FRONT, HIGH);
    digitalWrite(PIN_LEFT_LOW_FRONT, HIGH);
    digitalWrite(PIN_LEFT_REAR, HIGH);
    digitalWrite(PIN_RIGHT_REAR, HIGH);
    lighState |= LIGHT_ON;
  }
}

void LightsHigh(void)
{
  if( lighState == LIGHT_OFF )
  {
    LightsOn();
  }
Serial.println("Lights high");
  digitalWrite(PIN_RIGHT_HIGH_FRONT, HIGH);
  digitalWrite(PIN_LEFT_HIGH_FRONT, HIGH);
  lighState |= LIGHT_HIGH;
}

void LightsLow(void)
{
  if( lighState == LIGHT_OFF )
  {
    LightsOn();
  }
Serial.println("Lights low");
  digitalWrite(PIN_RIGHT_HIGH_FRONT, LOW);
  digitalWrite(PIN_LEFT_HIGH_FRONT, LOW);
  lighState &= LIGHT_HIGH;
}

void LightsOff(void)
{
  if( lighState != LIGHT_OFF )
  {
Serial.println("All lights off");
    digitalWrite(PIN_RIGHT_LOW_FRONT, LOW);
    digitalWrite(PIN_RIGHT_HIGH_FRONT, LOW);
    digitalWrite(PIN_LEFT_LOW_FRONT, LOW);
    digitalWrite(PIN_LEFT_HIGH_FRONT, LOW);
    digitalWrite(PIN_LEFT_REAR, LOW);
    digitalWrite(PIN_RIGHT_REAR, LOW);
    lighState = LIGHT_OFF;
  }
}

// check light functionality
// remove ifdefs if desired
//
void lighTest()
{

#ifdef NODEF

  delay(1000);
Serial.println("All lights off");
  LightsOff();
  delay(5000);
Serial.println("Lights on");
  LightsOn();
Serial.println("Right turn on");
  flashTurn( PIN_RIGHT_TURN_FRONT, PIN_RIGHT_TURN_REAR, ON );
  delay(5000);
Serial.println("Right turn off");
  flashTurn( PIN_RIGHT_TURN_FRONT, PIN_RIGHT_TURN_REAR, OFF );
  delay(1000);
Serial.println("Left turn on");
  flashTurn( PIN_LEFT_TURN_FRONT, PIN_LEFT_TURN_REAR, ON );
  delay(5000);
Serial.println("Left turn off");
  flashTurn( PIN_LEFT_TURN_FRONT, PIN_LEFT_TURN_REAR, OFF );
  delay(1000);
Serial.println("High lights on");
LightsHigh();
  delay(5000);
Serial.println("High lights off");
LightsLow();
  delay(5000);
Serial.println("Brake lights on");
flashBreakLights( PIN_RIGHT_BRAKE_REAR, PIN_LEFT_BRAKE_REAR, ON );
  delay(5000);
Serial.println("Brake lights off");
flashBreakLights( PIN_RIGHT_BRAKE_REAR, PIN_LEFT_BRAKE_REAR, OFF );
  delay(5000);
Serial.println("Lights on");  
  LightsOn();
Serial.println("Brake lights on");
flashBreakLights( PIN_RIGHT_BRAKE_REAR, PIN_LEFT_BRAKE_REAR, ON );
  delay(5000);
Serial.println("Brake lights off");
flashBreakLights( PIN_RIGHT_BRAKE_REAR, PIN_LEFT_BRAKE_REAR, OFF );
  delay(5000);
Serial.println("Lights high");  
LightsHigh();
Serial.println("Brake lights on");
flashBreakLights( PIN_RIGHT_BRAKE_REAR, PIN_LEFT_BRAKE_REAR, ON );
  delay(5000);
Serial.println("Brake lights off");
flashBreakLights( PIN_RIGHT_BRAKE_REAR, PIN_LEFT_BRAKE_REAR, OFF );
  delay(5000);
Serial.println("Lights off");  
LightsOff();
Serial.println("Brake lights on");
flashBreakLights( PIN_RIGHT_BRAKE_REAR, PIN_LEFT_BRAKE_REAR, ON );
  delay(5000);
Serial.println("Brake lights off");
flashBreakLights( PIN_RIGHT_BRAKE_REAR, PIN_LEFT_BRAKE_REAR, OFF );
  delay(4000);

Serial.println("All Front lights off");
  digitalWrite(PIN_RIGHT_LOW_FRONT, LOW);
  digitalWrite(PIN_RIGHT_HIGH_FRONT, LOW);
  digitalWrite(PIN_LEFT_LOW_FRONT, LOW);
  digitalWrite(PIN_LEFT_HIGH_FRONT, LOW);
  delay(3000);
Serial.println("Front lights on");
  digitalWrite(PIN_RIGHT_LOW_FRONT, HIGH);
  digitalWrite(PIN_LEFT_LOW_FRONT, HIGH);
  delay(3000);
Serial.println("Front lights off");
  digitalWrite(PIN_RIGHT_LOW_FRONT, LOW);
  digitalWrite(PIN_LEFT_LOW_FRONT, LOW);
Serial.println("Front high lights on");
  digitalWrite(PIN_RIGHT_HIGH_FRONT, HIGH);
  digitalWrite(PIN_LEFT_HIGH_FRONT, HIGH);
  delay(3000);
  
Serial.println("All Front lights off");
  digitalWrite(PIN_RIGHT_LOW_FRONT, LOW);
  digitalWrite(PIN_RIGHT_HIGH_FRONT, LOW);
  digitalWrite(PIN_LEFT_LOW_FRONT, LOW);
  digitalWrite(PIN_LEFT_HIGH_FRONT, LOW);
  delay(1000);

Serial.println("Right front high light on");
  digitalWrite(PIN_RIGHT_HIGH_FRONT, LOW);
  digitalWrite(PIN_LEFT_HIGH_FRONT, LOW);
  digitalWrite(PIN_RIGHT_HIGH_FRONT, HIGH);
  delay(3000);

Serial.println("Left front high light on");
  digitalWrite(PIN_RIGHT_HIGH_FRONT, LOW);
  digitalWrite(PIN_LEFT_HIGH_FRONT, LOW);
  digitalWrite(PIN_LEFT_HIGH_FRONT, HIGH);
  delay(3000);

  Serial.println("All Front lights off");
  digitalWrite(PIN_RIGHT_LOW_FRONT, LOW);
  digitalWrite(PIN_RIGHT_HIGH_FRONT, LOW);
  digitalWrite(PIN_LEFT_LOW_FRONT, LOW);
  digitalWrite(PIN_LEFT_HIGH_FRONT, LOW);
  delay(1000);

Serial.println("Right front turn flash");
  flashTurn( PIN_RIGHT_TURN_FRONT, 0, ON );
  delay(5000);
  flashTurn( PIN_RIGHT_TURN_FRONT, 0, OFF );
    
Serial.println("Left front turn flash");
  flashTurn( PIN_LEFT_TURN_FRONT, 0, ON );
  delay(5000);
  flashTurn( PIN_LEFT_TURN_FRONT, 0, OFF );
delay(3000);

Serial.println("Rear lights on");
  digitalWrite(PIN_LEFT_REAR, HIGH);
  digitalWrite(PIN_RIGHT_REAR, HIGH);
  delay(5000);
Serial.println("Rear lights off");
  digitalWrite(PIN_LEFT_REAR, LOW);
  digitalWrite(PIN_RIGHT_REAR, LOW);
  delay(1000);
Serial.println("Break lights on");
  digitalWrite(PIN_LEFT_BRAKE_REAR, HIGH);
  digitalWrite(PIN_RIGHT_BRAKE_REAR, HIGH);
  delay(5000);
Serial.println("Break lights off");
  digitalWrite(PIN_LEFT_BRAKE_REAR, LOW);
  digitalWrite(PIN_RIGHT_BRAKE_REAR, LOW);
  delay(1000);
Serial.println("Right turn flash on");
  flashTurn( PIN_RIGHT_TURN_FRONT, PIN_RIGHT_TURN_REAR, ON );
  delay(5000);
Serial.println("Right turn flash off");
  flashTurn( PIN_RIGHT_TURN_FRONT, PIN_RIGHT_TURN_REAR, OFF );
  delay(1000);
Serial.println("Left turn flash on");
  flashTurn( PIN_LEFT_TURN_FRONT, PIN_RIGHT_TURN_REAR, ON );
  delay(5000);
Serial.println("Left turn flash off");
  flashTurn( PIN_LEFT_TURN_FRONT, PIN_RIGHT_TURN_REAR, OFF );
  delay(1000);

Serial.println("Rotation clockwise light on");
  RotationLight( CLOCKWISE, ON );
  delay(5000);
Serial.println("Rotation clockwise light off");  
  RotationLight( CLOCKWISE, OFF );
Serial.println("Rotation counter-clockwise light on");
  RotationLight( CCLOCKWISE, ON );
  delay(5000);
Serial.println("Rotation counter-clockwise light off");
  RotationLight( CCLOCKWISE, OFF );
  delay(1000);
#endif // NODEF

}



// the loop function runs over and over again forever
void loop() {
  lighTest();

  doRotationLight();

}

