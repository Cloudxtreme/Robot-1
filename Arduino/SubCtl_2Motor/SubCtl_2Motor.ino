/*
  This is a simple motor controlling software for an Arduino Pro Mini
   
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

#define I2C_SLAVE_2MOTOR  0x0c
#define SERIAL_BAUD       57600
#define I2C_BUF_SIZE         32

volatile char i2cInBuf[I2C_BUF_SIZE];
char i2cOutBuf[I2C_BUF_SIZE];
volatile int i2cInIdx;

#define EEADDR_I2CSLAVE     0
#define EEADDR_VALIDDATA   26

#define EEPROM_VALIDATION  99

#define PINGDIR_FRONT    'f'
#define PINGDIR_BACK     'b'
#define PINGDIR_RIGHT    'r'
#define PINGDIR_LEFT     'l'

#define RESPONSE_OK       '0'
#define RESPONSE_FAIL     '2'
#define RESPONSE_IGNORED  '3'

#define REASON_UNKNOWN    '?'
#define REASON_NOT_APPL   '9'
#define pin2Master       2

//
// ISRs
//
// the isr for the right side
//
void rightISR(void)
{
  rightEncoder->pCallback();
}
//
// the isr for the left side
//
void leftISR(void)
{
  leftEncoder->pCallback();
}
//
// timer isr for synchronize dc motors
//
void timerIsr(void) 
{
  pEngine->syncService();
}
//
// ////////////////////////////////////////////////////////////////////
//
// ---------------------------------------------------------
// read an integer value from EEPROM
// ---------------------------------------------------------
int readIntEEPROM( int addr )
{

  uint8_t highByte;
  uint8_t lowByte;
  int result;

  highByte = EEPROM.read(addr);
  lowByte = EEPROM.read(addr+1);  
  result = (highByte << 8) + lowByte;
  
  return( result );
}

// ---------------------------------------------------------
// store an integer value to EEPROM
// ---------------------------------------------------------
void storeIntEEPROM( int addr, int value )
{

  uint8_t highByte;
  uint8_t lowByte;

  highByte = (value >> 8) &0xff;
  lowByte = value &0xff;
  
  EEPROM.write(addr, highByte);
  EEPROM.write(addr+1, lowByte);    
}

// ---------------------------------------------------------
// read a long value from EEPROM
// ---------------------------------------------------------
long readLongEEPROM( int addr )
{

  uint16_t highWord;
  uint16_t lowWord;
  long result;

  highWord = readIntEEPROM(addr);
  lowWord = readIntEEPROM(addr+2);
  
  result = (highWord << 16) + lowWord;
  
  return( result );
}

// ---------------------------------------------------------
// store a long value to EEPROM
// ---------------------------------------------------------
void storeLongEEPROM( int addr, long value )
{

  uint16_t highWord;
  uint16_t lowWord;

  highWord = (value >> 16) &0xffff;
  lowWord = value &0xffff;
  
  storeIntEEPROM( addr, highWord );
  storeIntEEPROM( addr+2, lowWord );
  
}

// ---------------------------------------------------------
// store control vars to EEPROM
// ---------------------------------------------------------
void storeEEPROM()
{
  storeIntEEPROM( EEADDR_VALIDDATA, EEPROM_VALIDATION);
}

// ---------------------------------------------------------
// read control vars from EEPROM
// ---------------------------------------------------------
int readEEPROM()
{
  int Validation; 
  int retval = -1;

  if( (Validation = 
       readIntEEPROM( EEADDR_VALIDDATA )) == EEPROM_VALIDATION )
  {
    retval = 0;
  }

  return(retval);
}

// ---------------------------------------------------------
// check pointer before parsing
// int checkPtr(void* inp, void* fmt, void* token, void* arg)
// ---------------------------------------------------------
int checkPtr(void* inp, void* fmt, void* token, void* arg)
{
  int rc = -1;

  if( inp != NULL && fmt != NULL && token != NULL && arg != NULL )
  {
    rc = 0;
  }
  
  return(rc);
}

// ---------------------------------------------------------
// parse for one integer Argument e.g. set interval
// int parseOneArg(char* inp, char *fmt, char* token, int *arg)
// ---------------------------------------------------------
int parseOneIntArg(char* inp, char *fmt, char* token, int *arg)
{
  int rc = checkPtr( (void*) inp,(void*) fmt,
                       (void*) token,(void*) arg);
  
  if( rc == 0 )  
  {
    rc = sscanf(inp, fmt, token, arg);
  }
  
  return(rc);
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
// receive all data present on the I2C bus
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

long serialBaud = SERIAL_BAUD;
int i2cSlaveAddr = I2C_SLAVE_MOTOR;
int encPulsePerRevolution = ENC_PULSE_PER_REVOLUTION;
int defaultDutyCycle = 254;
int goLowerLimit = 1;
int goUpperLimit = 1;
int edge = FALLING;
#define DEFAULT_DIRECTION dirStopp
currDirection = DEFAULT_DIRECTION;
#define DEFAULT_SPEED 254
currSpeed = DEFAULT_SPEED;
int verboseLevel;
int debugLevel;
int userCmd;
#define DEFAULT_SYNC_STATUS
#define SYNC_STATUS_OFF
#define SYNC_STATUS_ON
#define SYNC_STATUS_AUTO

int syncStatus = DEFAULT_SYNC_STATUS;

int powerAll;
int powerLeft;
int powerRight;
long currRotationsRight
long currRotationsLeft
long currDistanceRight
long currDistanceLeft


// ---------------------------------------------------------
// softReset
// ---------------------------------------------------------
void softReset()
{
}

// ---------------------------------------------------------
// emergencyStopp
// ---------------------------------------------------------
void emergencyStopp()
{
}


// ---------------------------------------------------------
// i2cRequest callback - triggered on incoming request
// ---------------------------------------------------------
void i2cRequest(void)
{
  char ctoken;
  char xtoken;
  int cArg;
  static char responseBuffer[I2C_BUF_SIZE];
  
  if( strlen((char*) i2cInBuf) >= 2 )
  {
    if( i2cInBuf[0] == '%' )
    {
//      Serial.println( (char*) i2cInBuf );
      // valid request ... lookup which one
      switch( ctoken = i2cInBuf[1] )
      {
        case 'g':   // CMD2MOTOR_GO_UP Übergangslos beschleunigen auf max
           parseOneIntArg( (char*) i2cInBuf, "%%%c%%%d", &xtoken,
                           &cArg);
           currSpeed = cArg;
           sprintf(responseBuffer, "%%%c%%%c%%%d", xtoken,
                 RESPONSE_OK, currSpeed );
           break;
        case 'G':   // CMD2MOTOR_GO_DOWN Übergangslos abbremsen auf min
           parseOneIntArg( (char*) i2cInBuf, "%%%c%%%d", &xtoken,
                           &cArg);
           currSpeed = cArg;
           sprintf(responseBuffer, "%%%c%%%c%%%d", xtoken,
                 RESPONSE_OK, currSpeed );
           break;
        case 'd':   // CMD2MOTOR_SLOW_DOWN Langsam abbremsen auf min
           parseOneIntArg( (char*) i2cInBuf, "%%%c%%%d", &xtoken,
                           &cArg);
           goLowerLimit = cArg;
           sprintf(responseBuffer, "%%%c%%%c%%%d", xtoken,
                 RESPONSE_OK, goLowerLimit );
           break;
        case 'u':   // CMD2MOTOR_SPEED_UP Langsam beschleunigen auf max
           parseOneIntArg( (char*) i2cInBuf, "%%%c%%%d", &xtoken,
                           &cArg);
           goUpperLimit = cArg;
           sprintf(responseBuffer, "%%%c%%%c%%%d", xtoken,
                 RESPONSE_OK, goUpperLimit );
           break;
        case 'a':   // CMD2MOTOR_GO_AHEAD Fahrtrichtung setzen vorwaerts
           sprintf(responseBuffer, "%%%c%%%c%%%d", ctoken,
                   RESPONSE_OK, currDirection );
           currDirection = dir_forward;
           break;
        case 'R':   // CMD2MOTOR_GO_BACK Fahrtrichtung setzen rueckwaerts
           sprintf(responseBuffer, "%%%c%%%c%%%d", ctoken,
                   RESPONSE_OK, currDirection );
           currDirection = dir_backward;
           break;
        case 'l':   // CMD2MOTOR_TURN_LEFT Fahrtrichtung setzen links 0-90°
           parseOneIntArg( (char*) i2cInBuf, "%%%c%%%d", &xtoken,
                           &cArg);
           turnDegrees = cArg;
           currDirection = dir_left;
           sprintf(responseBuffer, "%%%c%%%c%%%d", xtoken,
                 RESPONSE_OK, turnDegrees );
           break;
        case 'r':   // CMD2MOTOR_TURN_RIGHT Fahrtrichtung setzen rechts 0-90°
           parseOneIntArg( (char*) i2cInBuf, "%%%c%%%d", &xtoken,
                           &cArg);
           turnDegrees = cArg;
           currDirection = dir_right;
           sprintf(responseBuffer, "%%%c%%%c%%%d", xtoken,
                 RESPONSE_OK, turnDegrees );
           break;
        case 'D':   // CMD2MOTOR_GET_DIRECTION Fahrtrichtung holen
           sprintf(responseBuffer, "%%%c%%%c%%%d", ctoken,
                   RESPONSE_OK, currDirection );
           break;
        case 'x':   // CMD2MOTOR_STOP Stop
           sprintf(responseBuffer, "%%%c%%%c%%%d", ctoken,
                   RESPONSE_OK, currDirection );
           currDirection = dir_stopp;
           break;
        case 'X':   // CMD2MOTOR_BREAK Bremsen
           sprintf(responseBuffer, "%%%c%%%c%%%d", ctoken,
                   RESPONSE_OK, currDirection );
           currDirection = dir_brakes;
           break;
        case 'W':   // CMD2MOTOR_STORE_VALUES Werte in EEPROM schreiben
           storeEEPROM();
           sprintf(responseBuffer, "%%%c%%%c", ctoken,
                   RESPONSE_OK );
           break;
        case 'L':   // CMD2MOTOR_RESTORE_VALUES Werte aus EEPROM lesen
           readEEPROM();
           sprintf(responseBuffer, "%%%c%%%c", ctoken,
                   RESPONSE_OK );
           break;
        case 'V':   // CMD2MOTOR_SET_VERBOSE Protokoll-Modus
           parseOneIntArg( (char*) i2cInBuf, "%%%c%%%d", &xtoken,
                           &cArg);
           verboseLevel = cArg;
           sprintf(responseBuffer, "%%%c%%%c%%%d", xtoken,
                 RESPONSE_OK, verboseLevel );
           break;
        case 'T':   // CMD2MOTOR_SET_DEBUG Debug Modus
           parseOneIntArg( (char*) i2cInBuf, "%%%c%%%d", &xtoken,
                           &cArg);
           debugLevel = cArg;
           sprintf(responseBuffer, "%%%c%%%c%%%d", xtoken,
                 RESPONSE_OK, verboseLevel );
           break;
        case 'N':   // CMD2MOTOR_SET_SERIALBAUD Baudrate rs232 setzen
           parseOneLongArg( (char*) i2cInBuf, "%%%c%%%d", &xtoken,
                           &lArg);
           serialBaud = lArg;
           sprintf(responseBuffer, "%%%c%%%c%%%l", xtoken,
                 RESPONSE_OK, serialBaud );
           break;
        case 'n':   // CMD2MOTOR_GET_SERIALBAUD Baudrate rs232 holen
           sprintf(responseBuffer, "%%%c%%%c%%%l", ctoken,
                 RESPONSE_OK, serialBaud );
           break;
        case 'Y':   // CMD2MOTOR_RESET_DEFAULTS auf defaults zurücksetzen
           sprintf(responseBuffer, "%%%c%%%c", ctoken,
                 RESPONSE_OK );
           resetDefaults();
           break;
        case 'u':   // CMD2MOTOR_USER user-Kommando
           parseOneIntArg( (char*) i2cInBuf, "%%%c%%%d", &xtoken,
                           &cArg);
           userCmd = cArg;
           sprintf(responseBuffer, "%%%c%%%c%%%d", xtoken,
                 RESPONSE_OK, userCmd );
           break;
        case 'S':   // CMD2MOTOR_SET_SYNC_ON synchron ein
           syncStatus = SYNC_STATUS_ON;
           sprintf(responseBuffer, "%%%c%%%c%%%d", ctoken,
                 RESPONSE_OK, syncStatus );
           break;
        case 'O':   // CMD2MOTOR_SET_SYNC_OFF synchron aus
           syncStatus = SYNC_STATUS_OFF;
           sprintf(responseBuffer, "%%%c%%%c%%%d", ctoken,
                 RESPONSE_OK, syncStatus );
           break;
        case 'A':   // CMD2MOTOR_SET_SYNC_AUTO synchron automatik
           syncStatus = SYNC_STATUS_AUTO;
           sprintf(responseBuffer, "%%%c%%%c%%%d", ctoken,
                 RESPONSE_OK, syncStatus );
           break;
        case 's':   // CMD2MOTOR_GET_SYNC hole synchron status
           sprintf(responseBuffer, "%%%c%%%c%%%d", ctoken,
                 RESPONSE_OK, syncStatus );
           break;
        case 'p':   // CMD2MOTOR_SET_POWER setze Geschwindigkeit 0-100%
           parseOneIntArg( (char*) i2cInBuf, "%%%c%%%d", &xtoken,
                           &cArg);
           powerAll = cArg;
           sprintf(responseBuffer, "%%%c%%%c%%%d", xtoken,
                 RESPONSE_OK, powerAll );
           break;
        case 'f':   // CMD2MOTOR_SET_POWER_R setze Geschw. rechts 0-100%
           parseOneIntArg( (char*) i2cInBuf, "%%%c%%%d", &xtoken,
                           &cArg);
           powerRight = cArg;
           sprintf(responseBuffer, "%%%c%%%c%%%d", xtoken,
                 RESPONSE_OK, powerRight );
           break;
        case 'F':   // CMD2MOTOR_SET_POWER_L setze Geschw. links 0-100%
           parseOneIntArg( (char*) i2cInBuf, "%%%c%%%d", &xtoken,
                           &cArg);
           powerLeft = cArg;
           sprintf(responseBuffer, "%%%c%%%c%%%d", xtoken,
                 RESPONSE_OK, powerLeft );
           break;
        case 'P':   // CMD2MOTOR_GET_POWER hole Geschwindigkeit
           sprintf(responseBuffer, "%%%c%%%c%%%d", ctoken,
                 RESPONSE_OK, powerAll );
           break;
        case 'z':   // CMD2MOTOR_GET_POWER_R hole Geschwindigkeit rechts
           sprintf(responseBuffer, "%%%c%%%c%%%d", ctoken,
                 RESPONSE_OK, powerRight );
           break;
        case 'Z':   // CMD2MOTOR_GET_POWER_L hole Geschwindigkeit links
           sprintf(responseBuffer, "%%%c%%%c%%%d", ctoken,
                 RESPONSE_OK, powerLeft );
           break;
        case 'q':   // CMD2MOTOR_GET_ROT_R hole Umdrehungen gesamt rechts
           sprintf(responseBuffer, "%%%c%%%c%%%l", ctoken,
                 RESPONSE_OK, currRotationsRight );
           break;
        case 'Q':   // CMD2MOTOR_GET_ROT_L hole Umdrehungen gesamt links
           sprintf(responseBuffer, "%%%c%%%c%%%l", ctoken,
                 RESPONSE_OK, currRotationsLeft );
           break;
        case 'm':   // CMD2MOTOR_GET_MOT_R hole zurückgelegte Strecke rechts
           sprintf(responseBuffer, "%%%c%%%c%%%l", ctoken,
                 RESPONSE_OK, currDistanceRight );
           break;
        case 'M':   // CMD2MOTOR_GET_MOT_L hole zurückgelegte Strecke links
           sprintf(responseBuffer, "%%%c%%%c%%%l", ctoken,
                 RESPONSE_OK, currDistanceLeft );
           break;
        case 'I':   // CMD2MOTOR_RESET_MOTION Entfernung und Umdrehungen zurücksetzen
           sprintf(responseBuffer, "%%%c%%%c", ctoken,
                 RESPONSE_OK );
           break;
        case 'c':   // CMD2MOTOR_SOFTRESET coldstart 
           sprintf(responseBuffer, "%%%c%%%c", ctoken,
                 RESPONSE_OK );
           softReset();
           break;
        case 'K':   // CMD2MOTOR_EXIT stop program
           sprintf(responseBuffer, "%%%c%%%c", ctoken,
                 RESPONSE_OK );
           emergencyStopp();
           break;
        default:
           sprintf(responseBuffer, "%%%c%%%c%%%d", ctoken,
                   RESPONSE_FAIL, REASON_UNKNOWN );        
           break;
      }
      i2cResponse((char*) responseBuffer);
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

// ---------------------------------------------------------
// setup - preset defaults
// ---------------------------------------------------------
void setup() 
{
  reset2defaults();

  if( readEEPROM() < 0 )
  {
    storeEEPROM();
  }

  // enable serial in-/output  
  Serial.begin(serialBaud);
  
  // setup IIC communication
  Wire.begin(i2cSlaveAddr); // join i2c bus
  Wire.onReceive(i2cReceive); // register event  
  Wire.onRequest(i2cRequest); // register request

  rightEncoder = new HC020K(rightEncoderPin, rightEncIrq, edge, 
      encPulsePerRevolution);
  rightMotor = new L298N( pinIn1, pinIn2, pinRightPWM );
  rightEncoder->attachISR(rightISR);

  leftEncoder = new HC020K(leftEncoderPin, leftEncIrq, edge,
      encPulsePerRevolution);
  leftMotor = new L298N( pinIn3, pinIn4, pinLeftPWM );
  leftEncoder->attachISR(leftISR);

  pEngine = new SyncMotor(rightMotor, rightEncoder, leftMotor, leftEncoder);
  pEngine->attachService(2, timerIsr);
  
  // initialize the LED as an output:
  pinMode(ledPin, OUTPUT);

  currDirection = DEFAULT_DIRECTION;
  currSpeed = DEFAULT_SPEED;

  pEngine->setDutyCycle(currSpeed);
}

// ---------------------------------------------------------
// toggle LED on / off with 200 ms delay
// ---------------------------------------------------------
void toggleLED()
{
  digitalWrite(ledPin, ON);
  delay(200);
  digitalWrite(ledPin, OFF);
  delay(200);
}

// ---------------------------------------------------------
// signalLED -> call toggle LED howmany times
// ---------------------------------------------------------
void signalLED(int howmany)
{
  int i;
  
  for(i=0; i < howmany; i++)
  {
    toggleLED();
  }
}

// ---------------------------------------------------------
// switch onboard LED to desired state
// ---------------------------------------------------------
void onboardLED(int state)
{
  digitalWrite(ledPin, state);
}

// ---------------------------------------------------------
// main loop ...
// ---------------------------------------------------------
void loop() 
{
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

