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

#define PIN_LEFT_FRONT_LOW    12
#define PIN_LEFT_FRONT_HIGH   13
#define PIN_LEFT_FRONT_TURN   11

#define PIN_RIGHT_FRONT_LOW   9
#define PIN_RIGHT_FRONT_HIGH  10
#define PIN_RIGHT_FRONT_TURN  8

#define PIN_LEFT_BACK         8
#define PIN_LEFT_BREAK_REAR   7
#define PIN_LEFT_BACK_TURN    6

#define PIN_RIGHT_BACK        3
#define PIN_RIGHT_BREAK_REAR  4
#define PIN_RIGHT_BACK_TURN   2

#define PIN_ROTATION_1       A0
#define PIN_ROTATION_2       A1
#define PIN_ROTATION_3       A2
#define PIN_ROTATION_4       A3


#define FLOAT_TRANS          5

#define ON  1
#define OFF 0

#define CLOCKWISE    1
#define CCLOCKWISE   2

int ledFlash1_Pin = 0;
int ledFlash2_Pin = 0;
int ledFlash2_Dir = 0;
int ledFlash3_Pin = 0;

//
// ------------------ INTERRUPT HANDLING -------------------
//
void timer1_Isr()
{
  if( ledFlash1_Pin != 0 )
  {
    digitalWrite(ledFlash1_Pin, digitalRead(ledFlash1_Pin) ^ 1);
  }
}
//
void timer3_Isr()
{
  if( ledFlash3_Pin != 0 )
  {
    digitalWrite(ledFlash3_Pin, digitalRead(ledFlash3_Pin) ^ 1);
  }
}
//


void timer2_Isr()
{
//
  
  switch(ledFlash2_Pin)
  {
    case PIN_ROTATION_1:
      if( ledFlash2_Dir == CLOCKWISE )
      {
        digitalWrite(PIN_ROTATION_2, digitalRead(ledFlash2_Pin) );
        delay(FLOAT_TRANS);
        digitalWrite(ledFlash2_Pin, digitalRead(ledFlash2_Pin) ^ 1);
        ledFlash2_Pin = PIN_ROTATION_2;
      }
      else
      {
        if( ledFlash2_Dir == CCLOCKWISE )
        {
          digitalWrite(PIN_ROTATION_4, digitalRead(ledFlash2_Pin) );
          delay(FLOAT_TRANS);
          digitalWrite(ledFlash2_Pin, digitalRead(ledFlash2_Pin) ^ 1);
          ledFlash2_Pin = PIN_ROTATION_4;
        }
      }
      break;
    case PIN_ROTATION_2:
      if( ledFlash2_Dir == CLOCKWISE )
      {
        digitalWrite(PIN_ROTATION_3, digitalRead(ledFlash2_Pin) );
        delay(FLOAT_TRANS);
        digitalWrite(ledFlash2_Pin, digitalRead(ledFlash2_Pin) ^ 1);
        ledFlash2_Pin = PIN_ROTATION_3;
      }
      else
      {
        if( ledFlash2_Dir == CCLOCKWISE )
        {
          digitalWrite(PIN_ROTATION_1, digitalRead(ledFlash2_Pin) );
          delay(FLOAT_TRANS);
          digitalWrite(ledFlash2_Pin, digitalRead(ledFlash2_Pin) ^ 1);
          ledFlash2_Pin = PIN_ROTATION_1;
        }
      }
      break;
    case PIN_ROTATION_3:
      if( ledFlash2_Dir == CLOCKWISE )
      {
        digitalWrite(PIN_ROTATION_4, digitalRead(ledFlash2_Pin) );
        delay(FLOAT_TRANS);
        digitalWrite(ledFlash2_Pin, digitalRead(ledFlash2_Pin) ^ 1);
        ledFlash2_Pin = PIN_ROTATION_4;
      }
      else
      {
        if( ledFlash2_Dir == CCLOCKWISE )
        {
          digitalWrite(PIN_ROTATION_2, digitalRead(ledFlash2_Pin) );
          delay(FLOAT_TRANS);
          digitalWrite(ledFlash2_Pin, digitalRead(ledFlash2_Pin) ^ 1);
          ledFlash2_Pin = PIN_ROTATION_2;
        }
      }
      break;
    case PIN_ROTATION_4:
      if( ledFlash2_Dir == CLOCKWISE )
      {
        digitalWrite(PIN_ROTATION_1, digitalRead(ledFlash2_Pin) );
        delay(FLOAT_TRANS);
        digitalWrite(ledFlash2_Pin, digitalRead(ledFlash2_Pin) ^ 1);
        ledFlash2_Pin = PIN_ROTATION_1;
      }
      else
      {
        if( ledFlash2_Dir == CCLOCKWISE )
        {
          digitalWrite(PIN_ROTATION_3, digitalRead(ledFlash2_Pin) );
          delay(FLOAT_TRANS);
          digitalWrite(ledFlash2_Pin, digitalRead(ledFlash2_Pin) ^ 1);
          ledFlash2_Pin = PIN_ROTATION_3;
        }
      }
      break;
    default:
      break;
  }
}
//
// ---------------- END INTERRUPT HANDLING -----------------
//


void setup() 
{
  Serial.begin(57600);

  pinMode(PIN_RIGHT_FRONT_LOW, OUTPUT);
  pinMode(PIN_RIGHT_FRONT_HIGH, OUTPUT);
  pinMode(PIN_RIGHT_FRONT_TURN, OUTPUT);

  pinMode(PIN_LEFT_FRONT_LOW, OUTPUT);
  pinMode(PIN_LEFT_FRONT_HIGH, OUTPUT);
  pinMode(PIN_LEFT_FRONT_TURN, OUTPUT);

  digitalWrite(PIN_RIGHT_FRONT_LOW, LOW);
  digitalWrite(PIN_RIGHT_FRONT_HIGH, LOW);
  digitalWrite(PIN_RIGHT_FRONT_TURN, LOW);

  digitalWrite(PIN_LEFT_FRONT_LOW, LOW);
  digitalWrite(PIN_LEFT_FRONT_HIGH, LOW);
  digitalWrite(PIN_LEFT_FRONT_TURN, LOW);

  pinMode(PIN_LEFT_BACK, OUTPUT);
  pinMode(PIN_LEFT_BREAK_REAR, OUTPUT);
  pinMode(PIN_LEFT_BACK_TURN, OUTPUT);

  pinMode(PIN_RIGHT_BACK, OUTPUT);
  pinMode(PIN_RIGHT_BREAK_REAR, OUTPUT);
  pinMode(PIN_RIGHT_BACK_TURN, OUTPUT);

  digitalWrite(PIN_LEFT_BACK, LOW);
  digitalWrite(PIN_LEFT_BREAK_REAR, LOW);
  digitalWrite(PIN_LEFT_BACK_TURN, LOW);

  digitalWrite(PIN_RIGHT_BACK, LOW);
  digitalWrite(PIN_RIGHT_BREAK_REAR, LOW);
  digitalWrite(PIN_RIGHT_BACK_TURN, LOW);

  pinMode(PIN_ROTATION_1, OUTPUT);
  pinMode(PIN_ROTATION_2, OUTPUT);
  pinMode(PIN_ROTATION_3, OUTPUT);
  pinMode(PIN_ROTATION_4, OUTPUT);

  digitalWrite(PIN_ROTATION_1, LOW);
  digitalWrite(PIN_ROTATION_2, LOW);
  digitalWrite(PIN_ROTATION_3, LOW);
  digitalWrite(PIN_ROTATION_4, LOW);

  ledFlash1_Pin = OFF;
  ledFlash2_Pin = OFF;
  ledFlash2_Dir = OFF;
  ledFlash3_Pin = OFF;

  Timer1.initialize(500000);
  Timer1.attachInterrupt(timer1_Isr);
  Timer1.stop();

  MsTimer2::set(50, timer2_Isr);
  MsTimer2::stop();

}
void flashTurn( int pin, int state )
{
  if( state == ON )
  {
    ledFlash1_Pin = pin;
    Timer1.start();
  }
  else
  {
    Timer1.stop();
    digitalWrite(pin, LOW);
    ledFlash1_Pin = 0;
  }
}

void flashBreakLights( int pin, int state )
{
  if( state == ON )
  {
    ledFlash3_Pin = pin;
  }
  else
  {
    digitalWrite(pin, LOW);
    ledFlash3_Pin = 0;
  }
}

void RotationLight( int direction, int state )
{  
  if( state == ON )
  {
    ledFlash2_Pin = PIN_ROTATION_1;
    digitalWrite(ledFlash2_Pin, HIGH);
    ledFlash2_Dir = direction;
    MsTimer2::start();
  }
  else
  {
    MsTimer2::stop();
    digitalWrite(PIN_ROTATION_1, LOW);
    digitalWrite(PIN_ROTATION_2, LOW);
    digitalWrite(PIN_ROTATION_3, LOW);
    digitalWrite(PIN_ROTATION_4, LOW);
    ledFlash2_Pin = OFF;
    ledFlash2_Dir = OFF;
  }
}

// the loop function runs over and over again forever
void loop() {
  delay(1000);

Serial.println("Front lights off");
  digitalWrite(PIN_RIGHT_FRONT_LOW, HIGH);
  digitalWrite(PIN_LEFT_FRONT_LOW, HIGH);
  delay(3000);
Serial.println("Front lights on");
  digitalWrite(PIN_RIGHT_FRONT_HIGH, HIGH);
  digitalWrite(PIN_LEFT_FRONT_HIGH, HIGH);
  delay(3000);
Serial.println("Front lights off");
  digitalWrite(PIN_RIGHT_FRONT_LOW, LOW);
  digitalWrite(PIN_RIGHT_FRONT_HIGH, LOW);
  digitalWrite(PIN_LEFT_FRONT_LOW, LOW);
  digitalWrite(PIN_LEFT_FRONT_HIGH, LOW);
  delay(1000);
Serial.println("Right front turn flash");
  flashTurn( PIN_RIGHT_FRONT_TURN, ON );
  delay(5000);
Serial.println("Left front turn flash");
  flashTurn( PIN_RIGHT_FRONT_TURN, OFF );
  flashTurn( PIN_LEFT_FRONT_TURN, ON );
  delay(5000);
  flashTurn( PIN_LEFT_FRONT_TURN, OFF );
  delay(1000);
Serial.println("Rear lights on");
  digitalWrite(PIN_LEFT_BACK, HIGH);
  digitalWrite(PIN_RIGHT_BACK, HIGH);
  delay(5000);
Serial.println("Rear lights off");
  digitalWrite(PIN_LEFT_BACK, LOW);
  digitalWrite(PIN_RIGHT_BACK, LOW);
  delay(1000);
Serial.println("Break lights on");
  digitalWrite(PIN_LEFT_BREAK_REAR, HIGH);
  digitalWrite(PIN_RIGHT_BREAK_REAR, HIGH);
  delay(5000);
Serial.println("Break lights off");
  digitalWrite(PIN_LEFT_BREAK_REAR, LOW);
  digitalWrite(PIN_RIGHT_BREAK_REAR, LOW);
  delay(1000);
Serial.println("Right rear turn flash");
  flashTurn( PIN_RIGHT_BACK_TURN, ON );
  delay(5000);
Serial.println("Right rear turn flash off");
  flashTurn( PIN_RIGHT_BACK_TURN, OFF );
  delay(1000);
Serial.println("Left rear turn flash");
  flashTurn( PIN_LEFT_BACK_TURN, ON );
  delay(5000);
Serial.println("Left rear turn flash off");
  flashTurn( PIN_LEFT_BACK_TURN, OFF );
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
}
