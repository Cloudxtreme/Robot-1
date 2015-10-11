/* 
 **************************************************
 * Pro Mini as a subcontroller for a Raspberry Pi
 * controlled robot vehicle.
 * The subcontroller first reads several settings
 * from its eeprom. If values are not initialised,
 * a setup over serial line has to be started.
 * Pro Mini wiring is as follows:
 * A4/A5 reserved for I2C communications
 * D13 reserved for signalling
 * 
 * D2 -> signal to master
 * D3 -> n/c 
 * D4 -> LED red
 * D5 -> LED yellow
 * D6 -> LED green
 * D7 -> LED blue
 * D8 -> SR04 Ultrasonic Trigger
 * D9 -> SR04 Ultrasonic Echo
 *
 * D10 -> SR04 Ultrasonic Echo 
 * D11 -> SR04 Ultrasonic Trigger
 * D12 -> LED blue
 * D13 -> LED green
 * A0  -> LED yellow
 * A1  -> LED red
 * A2  -> n/c
 * A3  -> n/c
 *
 * A4  -> IIC SCL
 * A5  -> IIC SDA
 *
 * EEPROM mapping
 * Addr
 *   0  I2C Slave address
 *   1  reserved
 *
 **************************************************
*/

//
// !!!! Interrupt-Pin for Signalling RPi is D2 !!!!
//


#include <Wire.h>
#include <EEPROM.h>


#define pinLED_R_Rear   4
#define pinLED_Y_Rear   5
#define pinLED_G_Rear   6
#define pinLED_B_Rear   7
#define pinTriggerRear  8
#define pinEchoRear     9


#define pinEchoFront    10 
#define pinTriggerFront 11
#define pinLED_B_Front  12
#define pinLED_G_Front  13
#define pinLED_Y_Front  14
#define pinLED_R_Front  15

#define pin2Master       2

#define COND_RED     'R'
#define COND_YELLOW  'Y'
#define COND_GREEN   'G'

char pingStatus;
int pingPause;
long distance;
int pingInterval;
char pingDirection;
int userCmd;
int cArg;
int LimitCritical;
int LimitWarning;
int LimitGreen;
int VerboseLevel;
int DebugStatus;

#define DEFAULT_VERBOSE_LEVEL    0
#define DEFAULT_DEBUG_STATUS     0
#define DEFAULT_LIMIT_CRITICAL  10
#define DEFAULT_LIMIT_WARNING   30
#define DEFAULT_LIMIT_GREEN     100


#define DEFAULT_PING_INTVAL  300  // interval in ms

#define PINGDIR_FRONT    'f'
#define PINGDIR_BACK     'b'
#define PINGDIR_RIGHT    'r'
#define PINGDIR_LEFT     'l'

#define RESPONSE_OK       '0'
#define RESPONSE_FAIL     '2'
#define RESPONSE_IGNORED  '3'

#define REASON_UNKNOWN    '?'
#define REASON_NOT_APPL   '9'


int LedOn;
const int LedOff = HIGH;

int LedAllOn;
const int LedAllOff = HIGH;

int i2cSlaveAddr;
long serialBaud;

int condRedCount;
int maxAlarmCount; 

#define DEFAULT_MAX_ALARM_CNT 3

#define I2C_SLAVE_2SONIC   0x0b
#define SERIAL_BAUD       57600
#define I2C_BUF_SIZE         32

volatile char i2cInBuf[I2C_BUF_SIZE];
char i2cOutBuf[I2C_BUF_SIZE];
volatile int i2cInIdx;

#define EEADDR_I2CSLAVE     0
#define EEADDR_SERIALBAUD   2
#define EEADDR_LEDON        6
#define EEADDR_LEDALLON     8
#define EEADDR_PINGDIR     10
#define EEADDR_PINGINTVAL  12
#define EEADDR_LIMCRIT     14
#define EEADDR_LIMWARN     16
#define EEADDR_LIMGREEN    18
#define EEADDR_VERBOSE     20
#define EEADDR_DEBUG       22
#define EEADDR_MAXALARM    24
#define EEADDR_VALIDDATA   26

#define EEPROM_VALIDATION  99

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
// switch master irq line to high
// ---------------------------------------------------------
void i2cMasterSignalOn(void)
{
  digitalWrite(pin2Master, HIGH);
}

// ---------------------------------------------------------
// switch master irq line back to low
// ---------------------------------------------------------
void i2cMasterSignalOff(void)
{
  digitalWrite(pin2Master, LOW);
}

// ---------------------------------------------------------
// hold high level on master irq for 10 ms
// ---------------------------------------------------------
void i2cAlarmMaster(void)
{  
  if( ++condRedCount >= maxAlarmCount ) 
  {
    i2cMasterSignalOn();
    delay(10);
    i2cMasterSignalOff();
  }
}

// ---------------------------------------------------------
// reset control vars to defaults
// ---------------------------------------------------------
void reset2defaults()
{
  i2cInIdx  = 0;
  LedOn     = HIGH;
  LedAllOn  = LOW;

  pingDirection = PINGDIR_FRONT;
  pingInterval = DEFAULT_PING_INTVAL;
  pingPause = 0;

  LimitCritical = DEFAULT_LIMIT_CRITICAL;
  LimitWarning = DEFAULT_LIMIT_WARNING;
  LimitGreen = DEFAULT_LIMIT_GREEN;
  VerboseLevel = DEFAULT_VERBOSE_LEVEL;
  DebugStatus = DEFAULT_DEBUG_STATUS;
  
  i2cSlaveAddr = I2C_SLAVE_2SONIC;
  serialBaud = SERIAL_BAUD;
  maxAlarmCount = DEFAULT_MAX_ALARM_CNT;  
}

// ---------------------------------------------------------
// read an integer value from EEPROM
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
// store an integer value to EEPROM
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
// store control vars to EEPROM
// ---------------------------------------------------------
void storeEEPROM()
{
  storeIntEEPROM( EEADDR_I2CSLAVE, i2cSlaveAddr);
  storeLongEEPROM( EEADDR_SERIALBAUD, serialBaud);
  storeIntEEPROM( EEADDR_LEDON, LedOn);
  storeIntEEPROM( EEADDR_LEDALLON, LedAllOn);
  storeIntEEPROM( EEADDR_PINGDIR, pingDirection);
  storeIntEEPROM( EEADDR_PINGINTVAL, pingInterval);
  storeIntEEPROM( EEADDR_LIMCRIT, LimitCritical);
  storeIntEEPROM( EEADDR_LIMWARN, LimitWarning);
  storeIntEEPROM( EEADDR_LIMGREEN, LimitGreen);
  storeIntEEPROM( EEADDR_VERBOSE, VerboseLevel);
  storeIntEEPROM( EEADDR_DEBUG, DebugStatus);
  storeIntEEPROM( EEADDR_MAXALARM, maxAlarmCount);
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
    i2cSlaveAddr = readIntEEPROM( EEADDR_I2CSLAVE );
    serialBaud = readLongEEPROM( EEADDR_SERIALBAUD );
    LedOn = readIntEEPROM( EEADDR_LEDON );
    LedAllOn = readIntEEPROM( EEADDR_LEDALLON );
    pingDirection = readIntEEPROM( EEADDR_PINGDIR );
    pingInterval = readIntEEPROM( EEADDR_PINGINTVAL );
    LimitCritical = readIntEEPROM( EEADDR_LIMCRIT );
    LimitWarning = readIntEEPROM( EEADDR_LIMWARN );
    LimitGreen = readIntEEPROM( EEADDR_LIMGREEN );
    VerboseLevel = readIntEEPROM( EEADDR_VERBOSE );
    DebugStatus = readIntEEPROM( EEADDR_DEBUG );
    maxAlarmCount = readIntEEPROM( EEADDR_MAXALARM );  
  }

  return(retval);
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
//      Serial.println( (char*) i2cInBuf );
      // valid request ... lookup which one
      switch( ctoken = i2cInBuf[1] )
      {
        case 'd':  // CMD2SONIC_GET_DIRECTION
          sprintf(responseBuffer, "%%%c%%%c%%%d", ctoken,
                  RESPONSE_OK, pingDirection );
          break;
        case 'D':  // CMD2SONIC_GET_DISTANCE
          sprintf(responseBuffer, "%%%c%%%c%%%d", ctoken,
                  RESPONSE_OK, (int) distance );
          break;
        case 'C':  // CMD2SONIC_GET_CONDITION
          sprintf(responseBuffer, "%%%c%%%c%%%d", ctoken,
                  RESPONSE_OK, pingStatus );
          condRedCount = 0;
          break;
        case 'A':  // CMD2SONIC_SET_COND_R_DIST
          parseOneIntArg( (char*) i2cInBuf, "%%%c%%%d", &xtoken,
                          &cArg);
          LimitCritical = cArg;
          sprintf(responseBuffer, "%%%c%%%c%%%d", xtoken,
                  RESPONSE_OK, LimitCritical );
          break;
        case 'W':  // CMD2SONIC_SET_COND_Y_DIST
          parseOneIntArg( (char*) i2cInBuf, "%%%c%%%d", &xtoken,
                          &cArg);
          LimitWarning = cArg;
          sprintf(responseBuffer, "%%%c%%%c%%%d", xtoken,
                  RESPONSE_OK, LimitWarning );
          break;
        case 'G':  // CMD2SONIC_SET_COND_G_DIST
          parseOneIntArg( (char*) i2cInBuf, "%%%c%%%d", &xtoken,
                          &cArg);
          LimitGreen = cArg;
          sprintf(responseBuffer, "%%%c%%%c%%%d", xtoken,
                  RESPONSE_OK, LimitGreen );
          break;
        case 'a':  // CMD2SONIC_GET_COND_R_DIST
          sprintf(responseBuffer, "%%%c%%%c%%%d", ctoken,
                  RESPONSE_OK, LimitCritical );
          break;
        case 'w':  // CMD2SONIC_GET_COND_Y_DIST
          sprintf(responseBuffer, "%%%c%%%c%%%d", ctoken,
                  RESPONSE_OK, LimitWarning );
          break;
        case 'g':  // CMD2SONIC_GET_COND_G_DIST
          sprintf(responseBuffer, "%%%c%%%c%%%d", ctoken,
                  RESPONSE_OK, LimitGreen );
          break;
        case 'f':  // CMD2SONIC_SET_DIR_FORWARD
          pingDirection = PINGDIR_FRONT;
          sprintf(responseBuffer, "%%%c%%%c", ctoken,
                  RESPONSE_OK );
          break;
        case 'b':  // CMD2SONIC_SET_DIR_BACK
          pingDirection = PINGDIR_BACK;
          sprintf(responseBuffer, "%%%c%%%c", ctoken,
                  RESPONSE_OK );
          break;
        case 'i':  // CMD2SONIC_SET_INTERVALL
          sprintf(responseBuffer, "%%%c%%%c%%%d", ctoken,
                  RESPONSE_OK, pingInterval );
          break;
        case 'I':  // CMD2SONIC_GET_INTERVALL
          sprintf(responseBuffer, "%%%c%%%c%%%d", ctoken,
                  RESPONSE_OK, pingInterval );
          break;
        case 'r':  // CMD2SONIC_SET_DIR_RIGHT
          // pingDirection = PINGDIR_RIGHT;
          sprintf(responseBuffer, "%%%c%%%c%%%d", ctoken,
                  RESPONSE_IGNORED, REASON_NOT_APPL );        
          break;
        case 'l':  // CMD2SONIC_SET_DIR_LEFT
          // pingDirection = PINGDIR_LEFT;
          sprintf(responseBuffer, "%%%c%%%c%%%d", ctoken,
                  RESPONSE_IGNORED, REASON_NOT_APPL );        
          break;
        case 'u':  // CMD2SONIC_USER
          parseOneIntArg( (char*) i2cInBuf, "%%%c%%%d", &xtoken,
                          &userCmd);
          sprintf(responseBuffer, "%%%c%%%c%%%d", ctoken,
                  RESPONSE_OK, xtoken );
          break;
        case 's':  // CMD2SONIC_STOP
          sprintf(responseBuffer, "%%%c%%%c", ctoken,
                  RESPONSE_IGNORED );
          break;
        case 'p':  // CMD2SONIC_PAUSE
          pingPause = 1;
          sprintf(responseBuffer, "%%%c%%%c", ctoken,
                  RESPONSE_OK );
          break;
        case 'c':  // CMD2SONIC_CONTINUE
          pingPause = 0;
          sprintf(responseBuffer, "%%%c%%%c", ctoken,
                  RESPONSE_OK );
          break;
        case 'x':  // CMD2SONIC_EXIT
          sprintf(responseBuffer, "%%%c%%%c", ctoken,
                  RESPONSE_IGNORED );
          break;          
        case 'S':  // CMD2SONIC_STORE_VALUES
          storeEEPROM();
          sprintf(responseBuffer, "%%%c%%%c", ctoken,
                  RESPONSE_OK );
          break;
        case 'V':  // CMD2SONIC_SET_VERBOSE
          parseOneIntArg( (char*) i2cInBuf, "%%%c%%%d", &xtoken,
                          &cArg);
          VerboseLevel = cArg;
          sprintf(responseBuffer, "%%%c%%%c%%%d", ctoken,
                  RESPONSE_OK, VerboseLevel );
          break;
        case 'H':  // CMD2SONIC_SET_DIRECTION
          parseOneIntArg( (char*) i2cInBuf, "%%%c%%%d", &xtoken,
                          &cArg);
          pingDirection = cArg;
          sprintf(responseBuffer, "%%%c%%%c%%%d", ctoken,
                  RESPONSE_OK, pingDirection );
          break;
        case 'P':  // CMD2SONIC_SET_DISTANCE
          parseOneIntArg( (char*) i2cInBuf, "%%%c%%%d", &xtoken,
                          &cArg);
          distance = (long) cArg;
          sprintf(responseBuffer, "%%%c%%%c%%%d", ctoken,
                  RESPONSE_OK, (int) distance );
          break;
        case 'Z':  // CMD2SONIC_SET_CONDITION
          parseOneIntArg( (char*) i2cInBuf, "%%%c%%%d", &xtoken,
                          &cArg);
          pingStatus = cArg;
          sprintf(responseBuffer, "%%%c%%%c%%%d", ctoken,
                  RESPONSE_OK, pingStatus );
          break;
        case 'T':  // CMD2SONIC_SET_DEBUG
          parseOneIntArg( (char*) i2cInBuf, "%%%c%%%d", &xtoken,
                          &cArg);
          DebugStatus = cArg;
          sprintf(responseBuffer, "%%%c%%%c%%%d", ctoken,
                  RESPONSE_OK, DebugStatus );
          break;
        case 'R':  // CMD2SONIC_RESET_DEFAULTS
          reset2defaults();
          sprintf(responseBuffer, "%%%c%%%c", ctoken,
                  RESPONSE_OK );
          break;
        case 'n':  // CMD2SONIC_GET_MAXALARM
          sprintf(responseBuffer, "%%%c%%%c%%%d", ctoken,
                  RESPONSE_OK, maxAlarmCount );
          break;          
        case 'N':  // CMD2SONIC_SET_MAXALARM
          parseOneIntArg( (char*) i2cInBuf, "%%%c%%%d", &xtoken,
                          &cArg);
          maxAlarmCount = cArg;
          sprintf(responseBuffer, "%%%c%%%c%%%d", ctoken,
                  RESPONSE_OK, maxAlarmCount );
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
// send response to calling part
// ---------------------------------------------------------
void i2cResponse( char* msg )
{
  sprintf(i2cOutBuf, "%s", msg);
  Wire.write((uint8_t*) i2cOutBuf, strlen(i2cOutBuf)+1);   
}


// ---------------------------------------------------------
// setup
// ---------------------------------------------------------
void setup() {

  reset2defaults();

  if( readEEPROM() < 0 )
  {
    storeEEPROM();
  }

  // enable serial in-/output  
  Serial.begin(serialBaud);
  
  // reset globals
  condRedCount = 0;
  
  // setup IIC communication
  Wire.begin(i2cSlaveAddr); // join i2c bus
  Wire.onReceive(i2cReceive); // register event  
  Wire.onRequest(i2cRequest); // register request

  // setup pin modes
  pinMode(pinLED_R_Rear,  OUTPUT);
  pinMode(pinLED_Y_Rear,  OUTPUT);
  pinMode(pinLED_G_Rear,  OUTPUT);
  pinMode(pinLED_B_Rear,  OUTPUT);
  pinMode(pinLED_B_Front, OUTPUT);
  pinMode(pinLED_G_Front, OUTPUT);
  pinMode(pinLED_Y_Front, OUTPUT);
  pinMode(pinLED_R_Front, OUTPUT);

  pinMode(pinTriggerRear, OUTPUT);
  pinMode(pinTriggerFront, OUTPUT);

  pinMode(pinEchoRear, INPUT);
  pinMode(pinEchoFront, INPUT);

  // intialize LEDs
  digitalWrite(pinLED_R_Rear,  LedAllOff);
  digitalWrite(pinLED_Y_Rear,  LedAllOff);
  digitalWrite(pinLED_G_Rear,  LedAllOff);
  digitalWrite(pinLED_B_Rear,  LedAllOff);
  digitalWrite(pinLED_B_Front, LedAllOff);
  digitalWrite(pinLED_G_Front, LedAllOff);
  digitalWrite(pinLED_Y_Front, LedAllOff);
  digitalWrite(pinLED_R_Front, LedAllOff);

  // master irq line
  pinMode(pin2Master, OUTPUT);
  digitalWrite(pin2Master, LOW);
  
}

// ---------------------------------------------------------
// send a ping and return distance in cm
// ---------------------------------------------------------
long ping(int trigger, int echo, int actled, int noactled )
{
  long currDist;
  long duration;

  digitalWrite(actled,   LedOn);    // switch on
  digitalWrite(noactled, LedOff);   // switch off

  digitalWrite(trigger, LOW);     //
  delayMicroseconds(2);           //
  digitalWrite(trigger, HIGH);    //
//
  delayMicroseconds(10);          //
  digitalWrite(trigger, LOW);     //
  duration = pulseIn(echo, HIGH); //
  // distance = (duration/2) / 36.9;
  currDist = (duration/2) / 29.1; //

  return(currDist); // give back calculated distance in cm
}

// ---------------------------------------------------------
// switch on/off status LEDs
// ---------------------------------------------------------
char setStatusLED( long distance, int rLED, int yLED, int gLED )
{
  char currStatus;
  
  if( distance <= LimitCritical )
  {
    currStatus = COND_RED;
    digitalWrite(rLED, LedAllOn);
    digitalWrite(yLED, LedAllOff);
    digitalWrite(gLED, LedAllOff);
  }
  else
  {
    if( distance <= LimitGreen )
    {
      currStatus = COND_YELLOW;
      digitalWrite(rLED, LedAllOff);
      digitalWrite(yLED, LedAllOn);
      digitalWrite(gLED, LedAllOff);
    }
    else
    {
      currStatus = COND_GREEN;
      digitalWrite(rLED, LedAllOff);
      digitalWrite(yLED, LedAllOff);
      digitalWrite(gLED, LedAllOn);
    }
  } 
  
  return( currStatus);  
}

// ---------------------------------------------------------
// main loop
// ---------------------------------------------------------
void loop() 
{
  switch( userCmd )
  {
    case '0':
      LedOn = HIGH;
      userCmd = '\0';
      break;
    case '1':
      LedOn = LOW;
      userCmd = '\0';
      break;
    case 'e':
    case 'E':
      LedAllOn = LOW;
      userCmd = '\0';
      break;
    case 'a':
    case 'A':
      LedAllOn = HIGH;
      userCmd = '\0';
      break;      
    default:
      break;
  }

  if( !pingPause )
  {
    switch( pingDirection )
    {      
      case PINGDIR_FRONT:
        distance = ping(pinTriggerFront, pinEchoFront,
                        pinLED_B_Front, pinLED_B_Rear);
        pingStatus = setStatusLED(distance, pinLED_R_Front,
                              pinLED_Y_Front, pinLED_G_Front);      
        break;
      case PINGDIR_BACK:
        distance = ping(pinTriggerRear, pinEchoRear,
                        pinLED_B_Rear, pinLED_B_Front);
        pingStatus = setStatusLED(distance, pinLED_R_Rear,
                              pinLED_Y_Rear, pinLED_G_Rear);      
        break;
      case PINGDIR_RIGHT:
        break;
      case PINGDIR_LEFT:
        break;
      default:
        break;
    }
        
    switch( pingStatus )
    {
      case COND_RED:
        i2cAlarmMaster();
        break;
      case COND_YELLOW:
        break;
      case COND_GREEN:
        break;
      default:
        break;
    }
  }
  delay(pingInterval);
}



