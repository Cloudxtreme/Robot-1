/*
 * **************************************************
 * prototype for a remote sensor controller
 * (C) 2015 Dirk Schanz aka dreamshader
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
 * **************************************************
*/

#include "stdio.h"
#include "telegram.h"
#ifdef RASPBERRYPI
#include "sensorman.h"
#include "dbsqlite.h"
#endif // RASPBERRYPI


int protocolDataTelegramProcessRStation( int rStation, int* responseCompleted, volatile unsigned char remoteStation[] )
{
  int retVal = STATION_STATUS_WAIT_RESPONSE;

  switch(remoteStation[rStation])
  {
    case REMOTE_STATION_OFFLINE:
printf("switch Station %2x from OFFLINE to ONLINE ...\n", rStation );
      remoteStation[rStation] = REMOTE_STATION_ONLINE;
      retVal = STATION_STATUS_NO_ACTION;
      break;
    case REMOTE_STATION_ONLINE:
printf("switch Station %2x from ONLINE to INFO_PENDING ...\n", rStation );
      remoteStation[rStation] = REMOTE_STATION_INFO_PENDING;
      retVal = STATION_STATUS_SEND_REQUEST;
      break;
    case REMOTE_STATION_UNKNOWN:
      retVal = STATION_STATUS_NO_ACTION;
      break;
    case REMOTE_STATION_INFO_PENDING:
      if( *responseCompleted )
      {
printf("switch Station %2x from INFO_PENDING to INFO_COMPLETE ...\n", rStation );
        remoteStation[rStation] = REMOTE_STATION_INFO_COMPLETED;
        retVal = STATION_STATUS_NO_ACTION;
      }
      break;
    case REMOTE_STATION_INFO_COMPLETED:
      retVal = STATION_STATUS_NO_ACTION;
      break;
    default:
      retVal = STATION_STATUS_NO_ACTION;
      break;
  }
  return(retVal);
}

//
// makeDataTelegram
//
int protocolMakeDataTelegram( struct sTgramParam *pParam, union uTdata *pData, unsigned char telegram[] )
{
  short value;
  short decimals;
  int retVal = -1;

  if( pParam != NULL && pData != NULL )
  {
    telegram[TGRAM_TPOS_MAGIC]          = TGRAM_DATA_MAGIC;
    telegram[TGRAM_TPOS_PAYLOAD_SIZE]   = TGRAM_DATA_PAYLOAD_SIZE;
    telegram[TGRAM_TPOS_TGRAM_TYPE]     = TGRAM_TYPE_DATA;
    telegram[TGRAM_DATA_TPOS_STATION_ID]     = pParam->station_id;
    telegram[TGRAM_DATA_TPOS_SENSOR_ID]   = pParam->sensor_id;
    telegram[TGRAM_DATA_TPOS_SENSOR_TYPE] = pParam->sensor_type;
    telegram[TGRAM_DATA_TPOS_TARGETSYSTEM_ID] = pParam->targetSystemId;
    telegram[TGRAM_DATA_TPOS_UNIT]           = pParam->data_unit;

    switch( telegram[TGRAM_DATA_TPOS_DATA_TYPE] = pParam->data_type )
    {
      case TGRAM_DATATYPE_BOOL:
        telegram[TGRAM_DATA_BOOL_VALUE_POS] = pData->boolData;
        retVal = 1;
        break;
      case TGRAM_DATATYPE_CHAR:
        telegram[TGRAM_DATA_CHAR_VALUE_POS] = pData->charData;
        retVal = 1;
        break;
      case TGRAM_DATATYPE_UCHAR:
        telegram[TGRAM_DATA_UCHAR_VALUE_POS] = pData->u_charData;
        retVal = 1;
        break;
      case TGRAM_DATATYPE_SHORT:
        telegram[TGRAM_DATA_SHORT_MSB_POS] = pData->shortData >> 8;
        telegram[TGRAM_DATA_SHORT_LSB_POS] = pData->shortData & 0x00FF;
        retVal = 1;
        break;
      case TGRAM_DATATYPE_USHORT:
        telegram[TGRAM_DATA_USHORT_MSB_POS] = pData->u_shortData >> 8;
        telegram[TGRAM_DATA_USHORT_LSB_POS] = pData->u_shortData & 0x00FF;
        retVal = 1;
        break;
      case TGRAM_DATATYPE_LONG:
        telegram[TGRAM_DATA_LONG_HIGH_WORD_MSB_POS] = 
           pData->longData >> 24;
        telegram[TGRAM_DATA_LONG_HIGH_WORD_LSB_POS] = 
           pData->longData >> 16;
        telegram[TGRAM_DATA_LONG_LOW_WORD_MSB_POS] = 
           pData->longData >> 8;
        telegram[TGRAM_DATA_LONG_LOW_WORD_LSB_POS] = 
           pData->longData & 0x00FF;
        retVal = 1;
        break;
      case TGRAM_DATATYPE_ULONG:
        telegram[TGRAM_DATA_ULONG_HIGH_WORD_MSB_POS] = 
           pData->u_longData >> 24;
        telegram[TGRAM_DATA_ULONG_HIGH_WORD_LSB_POS] = 
           pData->u_longData >> 16;
        telegram[TGRAM_DATA_ULONG_LOW_WORD_MSB_POS] = 
           pData->u_longData >> 8;
        telegram[TGRAM_DATA_ULONG_LOW_WORD_LSB_POS] = 
           pData->u_longData & 0x00FF;
        retVal = 1;
        break;
      case TGRAM_DATATYPE_FLOAT:
        value = (short) pData->floatData;
        decimals = (pData->floatData - (float)value) * 10000;
        telegram[TGRAM_DATA_FLOAT_MSB_VALUE_POS] = value >> 8;
        telegram[TGRAM_DATA_FLOAT_LSB_VALUE_POS] = value & 0x00FF;
        telegram[TGRAM_DATA_FLOAT_MSB_DECIMAL_POS] = decimals >> 8;
        telegram[TGRAM_DATA_FLOAT_LSB_DECIMAL_POS] = decimals & 0x00FF;
        retVal = 1;
        break;
      case TGRAM_DATATYPE_RAW:
        telegram[TGRAM_DATA_RAW_LEN_POS] = pData->raw_len;
        for( int i = 0; i < pData->raw_len &&
                        i < TGRAM_DATA_PAYLOAD_SIZE; i++ )
        {
          telegram[TGRAM_DATA_RAW_VALUE_POS + i] = 
            pData->raw_bytes[i];
        }
        retVal = 1;
        break;
      default:
        break;
    }
  }
  return(retVal);
}


void protocolDataTelegramInfo( unsigned char telegram[] )
{
  short value, decimals;

  for(int i = 0; i < TGRAM_DATA_TOTAL_SIZE; i++ )
  {
    printf("%x:", telegram[i]);
  }
  printf("\n");

  if( telegram[TGRAM_TPOS_MAGIC] == TGRAM_DATA_MAGIC &&
      telegram[TGRAM_TPOS_TGRAM_TYPE] == TGRAM_TYPE_DATA )
  {

    printf("telegram type: DATA\n");
    printf("magic: %2x (TGRAM_DATA_MAGIC)\n", telegram[TGRAM_TPOS_MAGIC] );
    printf("payload: %2x\n", telegram[TGRAM_TPOS_PAYLOAD_SIZE] );
    printf("station-id: %2x\n", telegram[TGRAM_DATA_TPOS_STATION_ID] );
    printf("sensor-id: %2x\n", telegram[TGRAM_DATA_TPOS_SENSOR_ID] );
    printf("targetSystemId: %2d\n", telegram[TGRAM_DATA_TPOS_TARGETSYSTEM_ID] );

    switch( telegram[TGRAM_DATA_TPOS_SENSOR_TYPE] )
    {
      case TGRAM_COMPONENT_DS18B20:
        printf("sensor-type: %2x (DS18B20)\n", telegram[TGRAM_DATA_TPOS_SENSOR_TYPE] );
        break;
      case TGRAM_COMPONENT_BMP160:
//      case TGRAM_COMPONENT_BMP085:
        printf("sensor-type: %2x (BMP085/BMP160)\n", telegram[TGRAM_DATA_TPOS_SENSOR_TYPE] );
        break;
      default:
        printf("sensor-type: %2x unknown\n", telegram[TGRAM_DATA_TPOS_SENSOR_TYPE] );
        break;
    }

    switch( telegram[TGRAM_DATA_TPOS_DATA_TYPE] )
    {
      case TGRAM_DATATYPE_BOOL:
        printf("data type: %2x (BOOL)\n", telegram[TGRAM_DATA_TPOS_DATA_TYPE] );
        break;
      case TGRAM_DATATYPE_CHAR:
        printf("data type: %2x (CHAR)\n", telegram[TGRAM_DATA_TPOS_DATA_TYPE] );
        break;
      case TGRAM_DATATYPE_UCHAR:
        printf("data type: %2x (UCHAR)\n", telegram[TGRAM_DATA_TPOS_DATA_TYPE] );
        break;
      case TGRAM_DATATYPE_SHORT:
        printf("data type: %2x (SHORT)\n", telegram[TGRAM_DATA_TPOS_DATA_TYPE] );
        break;
      case TGRAM_DATATYPE_USHORT:
        printf("data type: %2x (USHORT)\n", telegram[TGRAM_DATA_TPOS_DATA_TYPE] );
        break;
      case TGRAM_DATATYPE_LONG:
        printf("data type: %2x (LONG)\n", telegram[TGRAM_DATA_TPOS_DATA_TYPE] );
        break;
      case TGRAM_DATATYPE_ULONG:
        printf("data type: %2x (ULONG)\n", telegram[TGRAM_DATA_TPOS_DATA_TYPE] );
        break;
      case TGRAM_DATATYPE_FLOAT:
        printf("data type: %2x (FLOAT)\n", telegram[TGRAM_DATA_TPOS_DATA_TYPE] );
        value = telegram[TGRAM_DATA_FLOAT_MSB_VALUE_POS];
        value = value << 8;
        value |= telegram[TGRAM_DATA_FLOAT_LSB_VALUE_POS];
        decimals = telegram[TGRAM_DATA_FLOAT_MSB_DECIMAL_POS];
        decimals = decimals << 8;
        decimals |= telegram[TGRAM_DATA_FLOAT_LSB_DECIMAL_POS];
        printf("value: %d, decimals: %d \n", value, decimals );
        break;
      case TGRAM_DATATYPE_RAW:
        printf("data type: %2x (RAW)\n", telegram[TGRAM_DATA_TPOS_DATA_TYPE] );
        break;
      default:
        printf("data type: %2x (UNKNOWN)\n", telegram[TGRAM_DATA_TPOS_DATA_TYPE] );
        break;
    }

    switch( telegram[TGRAM_DATA_TPOS_UNIT] )
    {
      case TGRAM_UNIT_HPA:
        printf("unit: %2x (hPa)\n", telegram[TGRAM_DATA_TPOS_UNIT] );
        break;
      case TGRAM_UNIT_DGR_C:
        printf("unit: %2x (°C)\n", telegram[TGRAM_DATA_TPOS_UNIT] );
        break;
      case TGRAM_UNIT_DGR_F:
        printf("unit: %2x (°F)\n", telegram[TGRAM_DATA_TPOS_UNIT] );
        break;
      default:
        printf("unit: %2x (?)\n", telegram[TGRAM_DATA_TPOS_UNIT] );
        break;
    }
  }
}


int protocolMakeResponseTelegram( unsigned char station_id, unsigned char responseType, 
                                  unsigned char responseStatus, unsigned char targetSystemId,
                                  unsigned char telegram[], unsigned char payLoad[],
                                  unsigned char payLoadSize  )
{
  int retVal = 1;

  telegram[TGRAM_TPOS_MAGIC]                    = TGRAM_RESPONSE_MAGIC;
  telegram[TGRAM_TPOS_PAYLOAD_SIZE]             = TGRAM_RESPONSE_TPOS_RESPONSE_DATA  + payLoadSize;
  telegram[TGRAM_TPOS_TGRAM_TYPE]               = TGRAM_TYPE_RESPONSE;
  telegram[TGRAM_RESPONSE_TPOS_RESPONSE_TYPE]   = responseType;
  telegram[TGRAM_RESPONSE_TPOS_STATION_ID]      = station_id;
  telegram[TGRAM_RESPONSE_TPOS_RESPONSE_STATUS] = responseStatus;
  telegram[TGRAM_RESPONSE_TPOS_TARGETSYSTEM_ID] = targetSystemId;
  telegram[TGRAM_RESPONSE_TPOS_DATA_SIZE]       = payLoadSize;

  memcpy(&telegram[TGRAM_RESPONSE_TPOS_RESPONSE_DATA],
         payLoad, payLoadSize );     


  return(retVal);
}


int protocolMakeRequestTelegram( unsigned char station_id, unsigned char requestId, unsigned char telegram[] )
{
  int retVal = 1;

  telegram[TGRAM_TPOS_MAGIC]              = TGRAM_REQUEST_MAGIC;
  telegram[TGRAM_TPOS_PAYLOAD_SIZE]       = TGRAM_REQUEST_PAYLOAD_SIZE;
  telegram[TGRAM_TPOS_TGRAM_TYPE]         = TGRAM_TYPE_REQUEST;
  telegram[TGRAM_REQUEST_TPOS_REQUEST_ID] = requestId;
  telegram[TGRAM_REQUEST_TPOS_STATION_ID] = station_id;

  return(retVal);
}

int protocolIsResponseComplete( unsigned char telegram[] )
{
  int retVal = 1;

  if( telegram[TGRAM_RESPONSE_TPOS_RESPONSE_STATUS] ==
         PROTOCOL_RESPONSE_COMPLETE )
  {
    retVal = 1;
  }
  else
  {
    retVal = 0;
  }

  return(retVal);
}

void protocolResponseTelegramInfo( unsigned char telegram[] )
{
  int i;

  if( telegram[TGRAM_TPOS_MAGIC] == TGRAM_RESPONSE_MAGIC &&
      telegram[TGRAM_TPOS_TGRAM_TYPE] == TGRAM_TYPE_RESPONSE )
  {

    printf("telegram type: RESPONSE\n");
    printf("magic: %2x (TGRAM_RESPONSE_MAGIC)\n", telegram[TGRAM_TPOS_MAGIC] );
    printf("payload: %2x\n", telegram[TGRAM_TPOS_PAYLOAD_SIZE] );

    printf("response-type: %2x\n", telegram[TGRAM_RESPONSE_TPOS_RESPONSE_TYPE] );
    printf("station-id: %2x\n", telegram[TGRAM_RESPONSE_TPOS_STATION_ID] );
    printf("response-status: %2x\n", telegram[TGRAM_RESPONSE_TPOS_RESPONSE_STATUS] );
    printf("targetSystemId: %d\n", telegram[TGRAM_RESPONSE_TPOS_TARGETSYSTEM_ID]);
    printf("data-size: %2x\n", telegram[TGRAM_RESPONSE_TPOS_DATA_SIZE] );
    printf("sensor-type: %2x\n", telegram[TGRAM_RESPONSE_TPOS_SENSOR_TYPE] );
    printf("sensor-id: %2x\n", telegram[TGRAM_RESPONSE_TPOS_SENSOR_ID] );

    printf("data: ");
    for( i = 0; i < telegram[TGRAM_RESPONSE_TPOS_DATA_SIZE]; i++ )
    {
      printf("%2x ", telegram[TGRAM_RESPONSE_TPOS_RESPONSE_DATA+i]);
    }
    printf("\n");
  }
}


#ifdef RASPBERRYPI

int protocolProcessResponseTelegram( unsigned char telegram[] )
{
  int rc;
  int retVal = -1;
  const char *insertFmt = "index.php?app=shc&a&ajax=pushsensorvalues&spid=%d&sid=%c%c-%s&type=1&v1=%3.2f";

  retVal = -1;

  if( telegram[TGRAM_TPOS_MAGIC] == TGRAM_RESPONSE_MAGIC &&
      telegram[TGRAM_TPOS_TGRAM_TYPE] == TGRAM_TYPE_RESPONSE )
  {
    if( (rc = dbExistSensor(telegram[TGRAM_RESPONSE_TPOS_STATION_ID],
                      telegram[TGRAM_RESPONSE_TPOS_SENSOR_ID] )) > 0)
    {
      printf("Sensor exists!\n");
      retVal = 0;
    }
    else
    {
      if( rc == 0 )
      {
        switch( telegram[TGRAM_DATA_TPOS_TARGETSYSTEM_ID] )
        {
          case TARGET_SYSTEM_SHC:
printf("targetsystem is SHC\n");
            insertFmt = SHC_API_FORMAT;
            break;
          case TARGET_SYSTEM_EMONCMS:
printf("targetsystem is EMONCMS\n");
            insertFmt = EMONCMS_API_FORMAT;
            break;
          default:
            break;
        }

        printf("Sensor NOT FOUND in DB!\n");
        if( dbAddSensor(telegram[TGRAM_RESPONSE_TPOS_STATION_ID],
                        telegram[TGRAM_RESPONSE_TPOS_SENSOR_ID],
                        telegram[TGRAM_RESPONSE_TPOS_SENSOR_TYPE],
                        telegram[TGRAM_DATA_TPOS_TARGETSYSTEM_ID],
                        &telegram[TGRAM_RESPONSE_TPOS_RESPONSE_DATA+2],
                        (char*) insertFmt) == 0)
        {
          printf("Sensor ADDED to DB!\n");
          retVal = 0;
        }
        else
        {
          printf("Sensor ADDING FAILED!\n");
        }
      }
else
{
printf("dbExistSensor returns %d\n", rc);
}

    }
  }

  return( retVal );
}





int protocolDataTelegram2SHC( unsigned char telegram[], void *pSensorEntry )
{
  int retVal = -1;
  float fValue;
  short value, decimals;
  struct _sens_table *sensorPtr;
  char targetTelegram[1024];

  if( (sensorPtr = (struct _sens_table*) pSensorEntry) != NULL )
  {
    retVal = 0;

    switch( telegram[TGRAM_DATA_TPOS_DATA_TYPE] )
    {
      case TGRAM_DATATYPE_BOOL:
        break;
      case TGRAM_DATATYPE_CHAR:
        break;
      case TGRAM_DATATYPE_UCHAR:
        break;
      case TGRAM_DATATYPE_SHORT:
        break;
      case TGRAM_DATATYPE_USHORT:
        break;
      case TGRAM_DATATYPE_LONG:
        break;
      case TGRAM_DATATYPE_ULONG:
        break;
      case TGRAM_DATATYPE_FLOAT:
        printf("data type: %2x (FLOAT)\n", telegram[TGRAM_DATA_TPOS_DATA_TYPE] );
        value = telegram[TGRAM_DATA_FLOAT_MSB_VALUE_POS];
        value = value << 8;
        value |= telegram[TGRAM_DATA_FLOAT_LSB_VALUE_POS];
        decimals = telegram[TGRAM_DATA_FLOAT_MSB_DECIMAL_POS];
        decimals = decimals << 8;
        decimals |= telegram[TGRAM_DATA_FLOAT_LSB_DECIMAL_POS];
        printf("value: %d, decimals: %d \n", value, decimals );
fValue = (float) value + (decimals / 10000.0) ;
        break;
      case TGRAM_DATATYPE_RAW:
        break;
      default:
        break;
    }

    switch(sensorPtr->sensorType)
    {
      case REMOTE_SENSORTYPE_BMP085:
      case REMOTE_SENSORTYPE_DS1621:
        break;
      case REMOTE_SENSORTYPE_DS18B20:
        sprintf(targetTelegram,
                sensorPtr->targetURLFmt,
                sensorPtr->stationId,
                sensorPtr->sensor.oneWire.address[0],
                sensorPtr->sensor.oneWire.address[1],
                &sensorPtr->sensor.oneWire.address[2],
                fValue );
printf("API call is: %s\n", targetTelegram);
        break;
      default:
        break;
    }
  }

  return( retVal );
}

int protocolDataTelegram2EMONCMS( unsigned char telegram[], void *pSensorEntry )
{
  int retVal = -1;
  struct _sens_table *sensorPtr;

  if( (sensorPtr = (struct _sens_table*) pSensorEntry) != NULL )
  {
  }

  return( retVal );
}


int protocolProcessDataTelegram( unsigned char telegram[], void *pSensorEntry )
{
//  short value, decimals;
  int retVal = -1;
  struct _sens_table *sensorPtr;

  if( telegram[TGRAM_TPOS_MAGIC] == TGRAM_DATA_MAGIC &&
      telegram[TGRAM_TPOS_TGRAM_TYPE] == TGRAM_TYPE_DATA )
  {

    if( (sensorPtr = (struct _sens_table*) pSensorEntry) != NULL )
    {
      retVal = dbSelectSensor( telegram[TGRAM_DATA_TPOS_STATION_ID],
                               telegram[TGRAM_DATA_TPOS_SENSOR_ID],
                               pSensorEntry );

      // retVal is 0 on success
      if(retVal == 0)
      {
// printf("protocolProcessDataTelegram -> select ok\n");
        switch( sensorPtr->targetSystemId )
        {
          case TARGET_SYSTEM_SHC:
// printf("targetsystem is SHC\n");
            retVal = protocolDataTelegram2SHC( telegram, pSensorEntry );
            break;
          case TARGET_SYSTEM_EMONCMS:
// printf("targetsystem is EMONCMS\n");
            retVal = protocolDataTelegram2EMONCMS( telegram, pSensorEntry );
            break;
          case TARGET_SYSTEM_NATIVE_TCP:
// printf("targetsystem is NATIVE TCP\n");
            break;
          case TARGET_SYSTEM_NATIVE_UDP:
// printf("targetsystem is NATIVE UDP\n");
            break;
          default:
// printf("targetsystem is unknown\n");
            break;
        }
      }
    }
  }

  return( retVal );
}

#endif // RASPBERRYPI

