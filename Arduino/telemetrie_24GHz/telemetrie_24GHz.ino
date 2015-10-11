/*
 * **************************************************
 * prototype for a wifi sensor controller
 * (C) 2014 Dirk Schanz aka dreamshader
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * --------------------------------------------------
 * includes code from: 
 * http://playground.arduino.cc/Code/AvailableMemory
 * http://www.pjrc.com/teensy/td_libs_OneWire.html
 * https://github.com/milesburton/Arduino-Temperature-Control-Library/blob/master
 * https://github.com/adafruit/DHT-sensor-library/blob/master
 * https://github.com/adafruit/Adafruit-BMP085-Library
 * https://github.com/arduino/Arduino/blob/master/libraries/Wire
 * https://github.com/arduino/Arduino/blob/master/libraries/SoftwareSerial
 * https://github.com/arduino/Arduino/blob/master/libraries/EEPROM
 *
 * **************************************************
*/

//
// Include what we need ...
//

#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

#include "telegram.h"

//
// 1Wire --------------------------------------------
//

OneWire *p_oneWire;
DallasTemperature *p_sensors;

// Pin 0 and 1 reserved for Rx and Tx
#define MIN_1WIRE_PIN   2
// without A0 to A3
#define MAX_1WIRE_PIN  13
// include A0 to A3
// #define MAX_1WIRE_PIN  17

#define PIN_1WIRE_BUS  2

uint8_t n1Wire;
int16_t bus1WPin;
uint8_t has1Wire;

uint8_t probe1Wire(int16_t pin)
{
  byte addr[8];
  uint8_t retVal = 0;

  OneWire bus(pin);
  if ( bus.search(addr)) 
  {
    bus.reset_search();
    while(bus.search(addr)) 
    {
      if ( OneWire::crc8( addr, 7) != addr[7]) 
      {
          Serial.print("CRC is not valid!\n\r");
      }
      else
      {
        n1Wire++;
        for(int i = 0; i < sizeof(addr); i++ )
        { 
          Serial.print(addr[i], HEX);
          Serial.print(":");
        }
        Serial.println("");
      }
    }
    retVal = 1;
    bus.reset_search();
  }
  return( retVal );
}


float read1Wire(uint16_t idx)
{
  float t = 0.0;
  if( p_sensors != NULL )
  {
    p_sensors->requestTemperatures();
    t = p_sensors->getTempCByIndex(idx);
  }
  return( t );
}

//
// END 1Wire ----------------------------------------
//


//
// radio related:
//
// transtype=24GHZ_RADIO
//
// channel=0x4c
// cepin=9
// csnpin=10
// pipe0=0x00,0x00,0x00,0xF0,0xF0,0xF0,0xF0,0xE1
// pipe1=0x00,0x00,0x00,0xC2,0xC2,0xC2,0xC2,0xC2
// pipe2=0x00,0x00,0x00,0xC2,0xC2,0xC2,0xC2,0xC3
// pipe3=0x00,0x00,0x00,0xC2,0xC2,0xC2,0xC2,0xC4
// pipe4=0x00,0x00,0x00,0xC2,0xC2,0xC2,0xC2,0xC5
// pipe5=0x00,0x00,0x00,0xC2,0xC2,0xC2,0xC2,0xC6
// 
// pipe0=0x00,0x00,0x00,0xF0,0xF0,0xF0,0xF0,0xE1
// pipe1=0x00,0x00,0x00,0xF0,0xF0,0xF0,0xF0,0xD2
// pipe2=0x00,0x00,0x00,0xF0,0xF0,0xF0,0xF0,0xF1
// pipe3=0x00,0x00,0x00,0xF0,0xF0,0xF0,0xF0,0xC2
// pipe4=0x00,0x00,0x00,0xF0,0xF0,0xF0,0xF0,0xA1
// pipe5=0x00,0x00,0x00,0xF0,0xF0,0xF0,0xF0,0xB2


RF24 radio(9,10);
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };

//uint64_t pipe0;
//uint64_t pipe1;
//uint64_t pipe2;
//uint64_t pipe3;
//uint64_t pipe4;
//uint64_t pipe5;

//
// END 24GHZ
//


int ser_putc( char c, FILE* f) 
{
  Serial.write( c );
  return c;
} 

unsigned char stationId;

void setup()
{
  Serial.begin(57600);

  fdevopen( &ser_putc, NULL );

  stationId = 0x63;

  p_oneWire = NULL;
  bus1WPin = 0;
  p_sensors = NULL;
  n1Wire = 0;
  
  for(bus1WPin = MIN_1WIRE_PIN; 
    probe1Wire(bus1WPin) == 0 && bus1WPin < MAX_1WIRE_PIN; bus1WPin++)
  {
    ;
  } 
  if( n1Wire > 0 )
  {
    p_oneWire = new OneWire(bus1WPin);
    p_sensors = new DallasTemperature(p_oneWire);
    p_sensors->begin();
  }

//  pipe0=0xF0F0F0F0E1LL;
//  pipe1=0xF0F0F0F0D2LL;
  
  radio.begin();
  radio.setChannel(0x4c);
  radio.setAutoAck(1);
  radio.setRetries(15,15);                // Max delay between retries & number of retries
  

  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1,pipes[1]);

  radio.printDetails();
  radio.startListening();
  radio.stopListening();
}
//
// END SETUP -----------------------------------------
//



void send1WireTelegrams( struct sTgramParam  *pParam, union uTdata  *pData, unsigned char telegram[] )
{
  uint16_t idx1Wire;
  long msecs;
  char tmout;

  if( pParam != NULL && pData != NULL )
  {

    for( idx1Wire = 0; idx1Wire < n1Wire; idx1Wire++ )
    {
      pParam->sensor_id = idx1Wire;
      pData->floatData = read1Wire(idx1Wire);

      Serial.print(pData->floatData);
      Serial.println(" Grad Celsius");
    
      if( protocolMakeDataTelegram( pParam, pData, telegram ) > 0 )
      {
        msecs = millis();
        tmout = 0;
        while( !tmout && !radio.write( telegram, TGRAM_DATA_TOTAL_SIZE) )
        {
          if(millis() - msecs >= 1000)
          {
            tmout=1;
          }
        }
    
        if(tmout)
        {
          Serial.println("timeout sending temperature telegram");
        }
      }
      delay(25);
    }
  }
}


void sendResponseSensors( unsigned char telegram[] )
{
  uint16_t idx1Wire;
  int i, send_size;
  long msecs;
  char tmout;
  unsigned char responseStatus;
  unsigned char targetSystemId;
  unsigned char ds18b20Payload[10];
  unsigned char ds18b20AddrTemp[9];

  targetSystemId = TARGET_SYSTEM_SHC;
  ds18b20Payload[0] = REMOTE_SENSORTYPE_DS18B20;
  for( idx1Wire = 0; idx1Wire < n1Wire; idx1Wire++ )
  {
    ds18b20Payload[1] = (unsigned char) idx1Wire;
    p_sensors->getAddress(ds18b20AddrTemp, idx1Wire);

Serial.println("");
Serial.print("Raw buffer with W1Address:");    
for(int i = 0; i < sizeof(ds18b20AddrTemp);i++)
{
  Serial.print(ds18b20AddrTemp[i],HEX);
  Serial.print(":");
}  
Serial.println("");

    ds18b20Payload[2] = ds18b20AddrTemp[0];
    for(int j=1, i = sizeof(ds18b20AddrTemp)-3; i > 0; i--,j++ )
    {
      ds18b20Payload[2+j] = ds18b20AddrTemp[i];
    }  


Serial.println("");
Serial.print("W1Address:");    
for(int i = 2; i < sizeof(ds18b20Payload)-1;i++)
{
  Serial.print(ds18b20Payload[i],HEX);
  Serial.print(":");
}  
Serial.println("");

    if( idx1Wire + 1 < n1Wire )
    {
      responseStatus = PROTOCOL_RESPONSE_MORE;
    }
    else
    {
      responseStatus = PROTOCOL_RESPONSE_COMPLETE;
    }
    protocolMakeResponseTelegram( stationId, (unsigned char) PROTOCOL_RESPONSE_SENSORS, 
                                  responseStatus, targetSystemId, telegram,
                                  ds18b20Payload, (unsigned char) sizeof(ds18b20Payload)  );


    send_size = TGRAM_RESPONSE_TPOS_RESPONSE_DATA  + sizeof(ds18b20Payload);

    msecs = millis();
    tmout = 0;
    while( !tmout && !radio.write( telegram, send_size) )
    {
      if(millis() - msecs >= 1000)
      {
        tmout=1;
      }
    }
    
    if(tmout)
    {
      Serial.println("timeout sending response telegram");
    }

    delay(10);

 Serial.print("payload size ");
 Serial.println((unsigned char) sizeof(ds18b20Payload));

    Serial.print("RESPONSE to send: ");
    for( i = 0; i < send_size; i++ )
    {
      Serial.print(telegram[i],HEX);
      Serial.print(" ");
    }
    Serial.println("");

  }

}


void processRequest( unsigned char telegram[] )
{
  unsigned char station_id, requestId;
  int i;

  Serial.print("REQUEST received: :");
  for(i=0; i < TGRAM_REQUEST_TOTAL_SIZE; i++ )
  {
    Serial.print(telegram[i],HEX);
    Serial.print(":");
  }
  Serial.println("");

  if( (station_id = telegram[TGRAM_REQUEST_TPOS_STATION_ID]) == stationId )
  {
    switch( requestId = telegram[TGRAM_REQUEST_TPOS_REQUEST_ID] )
    {
      case PROTOCOL_REQUEST_SENSORS:
        Serial.println("Request sensors");
        sendResponseSensors( telegram );
        break;
      case PROTOCOL_REQUEST_UNKNOWN:
        Serial.println("Request type unknown");
        break;
      case PROTOCOL_REQUEST_NONE:
        Serial.println("Request type none");
        break;
      default:
        Serial.println("Request type no idea");
        break;
    }
  }
  else
  {
    Serial.print("Not mine - station requested is ");
    Serial.print(station_id, HEX);
    Serial.print(" my id is ");
    Serial.println(stationId, HEX);
  }
}

void processProt( unsigned char telegram[] )
{
}

void processCmd( unsigned char telegram[] )
{
}

void processQuery( unsigned char telegram[] )
{
}



//
// BEGIN LOOP ----------------------------------------
// 
void loop()
{

  //
  // 1Wire ...
  uint16_t idx1Wire;
  int i;
  static long lastCall;

  long msecs;
  char tmout;
  struct sTgramParam Param;
  union uTdata Data;
  unsigned char telegram[TGRAM_MAX_SIZE];
  int SensorInterval = 10000;

  Param.station_id = stationId;
  Param.sensor_type = TGRAM_COMPONENT_DS18B20;
  Param.targetSystemId = TARGET_SYSTEM_SHC;
  Param.data_unit = TGRAM_UNIT_DGR_C;
  Param.data_type = TGRAM_DATATYPE_FLOAT;

  if(lastCall == 0)
  {
    lastCall = millis();
    lastCall -= SensorInterval;
  }
  
  if( millis() - lastCall >= SensorInterval )
  {
    lastCall = millis();
    for( idx1Wire = 0; idx1Wire < n1Wire; idx1Wire++ )
    {
      Param.sensor_id = idx1Wire;
      Data.floatData = read1Wire(idx1Wire);

      Serial.print(Data.floatData);
      Serial.println(" Grad Celsius");
  
      if( protocolMakeDataTelegram( &Param, &Data, telegram ) > 0 )
      {
        radio.stopListening();
  
        msecs = millis();
        tmout = 0;
        while( !tmout && !radio.write( telegram, TGRAM_DATA_TOTAL_SIZE) )
        {
          if(millis() - msecs >= 1000)
          {
            tmout=1;
          }
        }
      
        if(tmout)
        {
          Serial.println("timeout sending temperature telegram");
        }
  
        radio.startListening();
      }
      else
      {
        Serial.print("Could not make telegram");
      }
    }
  }

  if ( radio.available() )
  {
    radio.stopListening();

    radio.read( telegram, sizeof(telegram) );

    if( telegram[TGRAM_TPOS_MAGIC] == TGRAM_REQUEST_MAGIC &&
        telegram[TGRAM_TPOS_TGRAM_TYPE] == TGRAM_TYPE_REQUEST )
    {
      processRequest( telegram );
    }

    if( telegram[TGRAM_TPOS_MAGIC] == TGRAM_PROT_MAGIC &&
        telegram[TGRAM_TPOS_TGRAM_TYPE] == TGRAM_TYPE_PROT )
    {
      processProt( telegram );
    }

    if( telegram[TGRAM_TPOS_MAGIC] == TGRAM_CMD_MAGIC &&
        telegram[TGRAM_TPOS_TGRAM_TYPE] == TGRAM_TYPE_CMD )
    {
      processCmd( telegram );
    }

    if( telegram[TGRAM_TPOS_MAGIC] == TGRAM_QUERY_MAGIC &&
        telegram[TGRAM_TPOS_TGRAM_TYPE] == TGRAM_TYPE_QUERY )
    {
      processQuery( telegram );
    }

    radio.startListening();

  }

  delay(50);
}

//
// typedef uint8_t DeviceAddress[8];
// DeviceAddress deviceAddress;
// getAddress(deviceAddress, deviceIndex);
//

