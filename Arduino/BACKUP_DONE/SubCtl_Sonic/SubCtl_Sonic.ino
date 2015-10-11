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
 * D2 -> Micro switch -> GND (use internal pullup!)
 * D4 -> SR04 Ultrasonic Irq output
 * D5 -> blue stepper
 * D6 -> pink stepper
 * D7 -> yellow stepper
 * D8 -> orange stepper
 * D9 -> SR04 Ultrasonic Trigger
 * D10 -> LED red
 * D11 -> LED yellow
 * D12 -> LED green
 *
 * EEPROM mapping
 * Addr
 *   0  I2C Slave address
 *   1  reserved
 *   2  HIGH byte stepper: steps per revolution
 *   3  LOW byte  stepper: steps per revolution
 *   4  L or R -> direction to step to neutral position
 *   5  reserved
 *   6  HIGH byte stepper: steps to neutral position
 *   7  LOW byte  stepper: steps to neutral position
 *   8  HIGH byte
 *   9  LOW byte
 *
 **************************************************
*/

//
// !!!! Interrupt-Pin for Signalling RPi is D2 !!!!
//


#include <Wire.h>
#include <EEPROM.h>
#include <AH_28BYJ48.h>

const int pinMicroSwitch = 2;
const int pinEcho = 4;
const int pinIn1Blue = 5;
const int pinIn2Pink = 6;
const int pinIn3Yellow = 7;
const int pinIn4Orange = 8;
const int pinTrigger = 9;
const int irq0 = 0;

const int pinLED_G = 12;
const int pinLED_Y = 11;
const int pinLED_R = 10;

const int pin2Master = 2;

const int stepsPerRevolution = 64*64;  // number of steps per revolution * gear factor
// EEPROM
// 64*64
// Anzahl Schritte links/rechts bis mittelstellung
// 
// initialize the library 
AH_28BYJ48 myStepper(stepsPerRevolution, pinIn1Blue, 
  pinIn2Pink, pinIn3Yellow, pinIn4Orange );

int addr = 0;
int i2cAddr;
int backsteps = 0;

#define I2C_SLAVE_SONIC  8
#define SERIAL_BAUD  57600
#define I2C_BUF_SIZE  32

volatile char i2cInBuf[I2C_BUF_SIZE];
char i2cOutBuf[I2C_BUF_SIZE];
volatile int i2cInIdx;
volatile boolean i2cMasterRequest;

long distance;


// ---------------------------------------------------------
// receive
// ---------------------------------------------------------

void i2cReceive(int howMany)
{
  while( Wire.available() ) // loop through all  
  {
    char c = Wire.read(); // receive byte as a character
    if( i2cInIdx < (I2C_BUF_SIZE - 1) ) // 14 because of loop all but last
    {
      i2cInBuf[i2cInIdx++] = c;
    }
  }
  i2cInBuf[i2cInIdx] = '\0';
  // Serial.println((char*) i2cInBuf); // print the request
  i2cMasterRequest = true;
  i2cInIdx = 0;
}


void i2cMasterSignalOn(void)
{
  digitalWrite(pin2Master, HIGH);
}

void i2cMasterSignalOff(void)
{
  digitalWrite(pin2Master, LOW);
}

void i2cAlarmMaster(void)
{
  i2cMasterSignalOn();
  delay(10);
  i2cMasterSignalOff();
}

void i2cRequest(void)
{
  sprintf(i2cOutBuf, "-> %ld cm", distance);
  Wire.write((uint8_t*) i2cOutBuf, strlen(i2cOutBuf)+1); 
}


// void i2cAlarm2Master( char* buf, int len )
// {
//  Wire.beginTransmission(4); // transmit to device #4  
//  Wire.send("x is "); // sends five bytes  
//  Wire.send(x); // sends one byte   
//  Wire.endTransmission(); // stop transmitting
//    Wire.beginTransmission(44); // transmit to device #44 (0x2c)
                              // device address is specified in datasheet
//  Wire.write(val);             // sends value byte  
//  Wire.endTransmission();     // stop transmitting

//  val++;        // increment value
//  if(val == 64) // if reached 64th position (max)
//  {
//    val = 0;    // start over from lowest value
//  }
//  delay(500);
// }
//
// Wire.endTransmission()
// Wire.endTransmission(stop)
// Parameters
//
// stop : boolean. true will send a stop message, releasing the bus after transmission. false will send a restart, keeping the connection active. 












// ---------------------------------------------------------
// isr microswitch
// ---------------------------------------------------------
volatile int switchFlag;

void microswitch(void)
{
  switchFlag = 1;
}


// ---------------------------------------------------------
// setup
// ---------------------------------------------------------


void setup() {

  switchFlag = 0;
  i2cInIdx = 0;
  i2cMasterRequest = false;

  Serial.begin(SERIAL_BAUD);
  
  Wire.begin(I2C_SLAVE_SONIC); // join i2c bus
  Wire.onReceive(i2cReceive); // register event  
  Wire.onRequest(i2cRequest); // register request
  
  i2cAddr = EEPROM.read(addr);
  Serial.print(" i2c addr read = ");
  Serial.println(i2cAddr);
  
  uint8_t v2 = EEPROM.read(addr+1);
  uint8_t v3 = EEPROM.read(addr+2);
  
  Serial.print(" read v2 = ");
  Serial.println(v2);
  Serial.print(" read v3 = ");
  Serial.println(v3);
  
  if( v2 == 255 && v3 == 255 )
  {
    v2 = 0x10;
    v3 = 0x00;
    EEPROM.write(addr+1, v2);
    EEPROM.write(addr+2, v3);
  }

  int xx = v2 << 8 + v3;

  Serial.print(" xx = ");
  Serial.println(xx);
  
  myStepper.setSpeedRPM(10);
  
  myStepper.step(200);
  delay(500);
  myStepper.step(-200);

//  pinMode(pinMicroSwitch, INPUT);
//  digitalWrite(pinMicroSwitch, HIGH); // pullup
//  attachInterrupt(irq0, microswitch, CHANGE); // RISING

  pinMode(pinEcho, INPUT);
  pinMode(pinTrigger, OUTPUT);

  pinMode(pinLED_R, OUTPUT);
  pinMode(pinLED_Y, OUTPUT);
  pinMode(pinLED_G, OUTPUT);

  digitalWrite(pinLED_R, HIGH);
  digitalWrite(pinLED_Y, HIGH);
  digitalWrite(pinLED_G, HIGH);
  
  pinMode(pin2Master, OUTPUT);
  digitalWrite(pin2Master, LOW);


#ifdef NEVERDEF
  int stepsdone = 0;
  while( switchFlag == 0 )
  {
    myStepper.step(1);
    stepsdone++;
  }
  delay(500);
  
  myStepper.step(-200);
  delay(500);
  myStepper.step(100);
  
  switchFlag = 0;  
  while( switchFlag == 0 )
  {
    myStepper.step(1);
    stepsdone++;
  }
  delay(500);
  myStepper.step(-300);
  delay(500);
  backsteps = 300;
#endif // NEVERDEF

// EEPROM.write(addr, i2cAddr);

}


// ---------------------------------------------------------
// loop
// ---------------------------------------------------------

void loop() {
// myStepper.setSpeedRPM(10);    
// myStepper.step(-stepsPerRevolution);
//Wire.beginTransmission(4); // transmit to device #4  
//Wire.send("x is "); // sends five bytes  
//Wire.send(x); // sends one byte   
//Wire.endTransmission(); // stop transmitting
  
  char c;
  long duration;
  static int rLED,yLED,gLED;
  int r, y, g;
  
  if( Serial.available() )
  {
  switch( c = Serial.read() )
  {
    case 'F':
    case 'f':
      myStepper.step(-1);
      backsteps++;
      Serial.println(backsteps);
      Serial.print(" > ");
      break;
    case 'Z':
    case 'z':
      myStepper.step(1);
      backsteps--;
      Serial.println(backsteps);
      Serial.print(" > ");
      break;
    case 'p':
    case 'P':
      Serial.println("PING!");
      digitalWrite(pinTrigger, LOW);
      delayMicroseconds(2);
      digitalWrite(pinTrigger, HIGH);
//
      delayMicroseconds(10);
      digitalWrite(pinTrigger, LOW);
      duration = pulseIn(pinEcho, HIGH);
      // distance = (duration/2) / 36.9;
      distance = (duration/2) / 29.1;
//      Serial.print("Duration: ");
//      Serial.print(duration);
//      Serial.print(", Distance: ");
//      Serial.println(distance); 
      if( distance <= 10 )
      {
        digitalWrite(pinLED_R, LOW);
        digitalWrite(pinLED_Y, HIGH);
        digitalWrite(pinLED_G, HIGH);
      }
      else
      {
        if( distance <= 100 && distance >= 30 )
        {
          digitalWrite(pinLED_R, HIGH);
          digitalWrite(pinLED_Y, LOW);
          digitalWrite(pinLED_G, HIGH);
        }
        else
        {
          digitalWrite(pinLED_R, HIGH);
          digitalWrite(pinLED_Y, HIGH);
          digitalWrite(pinLED_G, LOW);
        }
      }   
      break;
    case 'r':
    case 'R':
      rLED = !rLED;
      digitalWrite(pinLED_R, rLED);
      Serial.println("R");
      break;
    case 'y':
    case 'Y':
      yLED = !yLED;
      digitalWrite(pinLED_Y, yLED);
      Serial.println("Y");
      break;
    case 'g':
    case 'G':
      gLED = !gLED;
      digitalWrite(pinLED_G, gLED);
      Serial.println("G");
      break;
    case 'd':
    case 'D':
      for(r = 0; r < 255; r++)
      {
        analogWrite(pinLED_R, r);
        delay(5);
      }

      for(g = 0; g < 255; g++ )
      {
        analogWrite(pinLED_G, g);
        delay(5);
      }
      
      for(y = 0; y < 255; y++)
      {
        analogWrite(pinLED_Y, y);
        delay(5);
      }
      break;
  }
  }
  
  
//  Serial.println("PING!");
  digitalWrite(pinTrigger, LOW);
  delayMicroseconds(2);
  digitalWrite(pinTrigger, HIGH);
//
  delayMicroseconds(10);
  digitalWrite(pinTrigger, LOW);
  duration = pulseIn(pinEcho, HIGH);
  // distance = (duration/2) / 36.9;
  distance = (duration/2) / 29.1;
 // Serial.print("Duration: ");
 // Serial.print(duration);
 // Serial.print(", Distance: ");
 // Serial.println(distance); 
  if( distance <= 10 )
  {
 //   Serial.println("Cond red!");
    digitalWrite(pinLED_R, LOW);
    digitalWrite(pinLED_Y, HIGH);
    digitalWrite(pinLED_G, HIGH);
  }
  else
  {
    if( distance > 10 && distance <= 30 )
    {
//      Serial.println("Cond yellow!");
      i2cAlarmMaster();
      digitalWrite(pinLED_R, HIGH);
      digitalWrite(pinLED_Y, LOW);
      digitalWrite(pinLED_G, HIGH);
    }
    else
    {
//      Serial.println("Cond green!");
      digitalWrite(pinLED_R, HIGH);
      digitalWrite(pinLED_Y, HIGH);
      digitalWrite(pinLED_G, LOW);
    }
  }
  
  if( i2cMasterRequest == true )
  {
    Serial.print("I2C request: ");
    Serial.println((char*) i2cInBuf); // print the request
    i2cMasterRequest = false;
  }
  
  
  delay(100);
}



