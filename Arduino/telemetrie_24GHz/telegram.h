//
// include for radio wrapper class
//
#ifndef _TELEGRAM_H_
#define _TELEGRAM_H_

#ifndef RASPBERRYPI
#if (ARDUINO >= 100)
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif
#endif // RASPBERRYPI


#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef RASPBERRYPI
#define SHC_API_FORMAT "index.php?app=shc&a&ajax=pushsensorvalues&spid=%d&sid=%c%c-%s&type=1&v1=%3.2f"
#define EMONCMS_API_FORMAT "/emoncms/api/post?apikey=%s&json={temperature:%3.2f}"
#endif // RASPBERRYPI





#define REMOTE_SENSORTYPE_BMP085    7
#define REMOTE_SENSORTYPE_BMP160    7
#define REMOTE_SENSORTYPE_DS18B20   9
#define REMOTE_SENSORTYPE_DS1621   10


#define REMOTE_TRANS_MODE_ACTIVE       1
#define REMOTE_TRANS_MODE_PASSIVE      2

#define REMOTE_STATION_OFFLINE         0
#define REMOTE_STATION_ONLINE          1
#define REMOTE_STATION_UNKNOWN         2
#define REMOTE_STATION_INFO_PENDING    4
#define REMOTE_STATION_INFO_COMPLETED  8

#define STATION_STATUS_NO_ACTION       2
#define STATION_STATUS_WAIT_RESPONSE   3
#define STATION_STATUS_SEND_REQUEST    4

#define TARGET_SYSTEM_SHC              2
#define TARGET_SYSTEM_EMONCMS          3
#define TARGET_SYSTEM_NATIVE_TCP       4
#define TARGET_SYSTEM_NATIVE_UDP       5


struct sTgramParam {
  unsigned char station_id;
  unsigned char sensor_type;
  unsigned char targetSystemId;
  unsigned char data_unit;
  unsigned char sensor_id;
  unsigned char data_type;
};

union uTdata {
bool           boolData;
char           charData;
unsigned char  u_charData;
short          shortData;
unsigned short u_shortData;
long           longData;
unsigned long  u_longData;
float          floatData;
char           *raw_bytes;
int            raw_len;
};


#define TGRAM_DATATYPE_BOOL      1
#define TGRAM_DATATYPE_CHAR      2
#define TGRAM_DATATYPE_UCHAR     3
#define TGRAM_DATATYPE_SHORT     4
#define TGRAM_DATATYPE_USHORT    5
#define TGRAM_DATATYPE_LONG      6
#define TGRAM_DATATYPE_ULONG     7
#define TGRAM_DATATYPE_FLOAT     8
#define TGRAM_DATATYPE_RAW       9


#define TGRAM_UNIT_HPA           1
#define TGRAM_UNIT_DGR_C         2
#define TGRAM_UNIT_DGR_F         3
#define TGRAM_UNIT_METERS        4

#define TGRAM_DATA_MAGIC      0xaf
#define TGRAM_PROT_MAGIC      0xac
#define TGRAM_CMD_MAGIC       0xae
#define TGRAM_RESULT_MAGIC    0xab
#define TGRAM_QUERY_MAGIC     0xa7
#define TGRAM_INFO_MAGIC      0xad
#define TGRAM_REQUEST_MAGIC   0xaa
#define TGRAM_RESPONSE_MAGIC  0xa4


#define TGRAM_COMPONENT_BMP085	 REMOTE_SENSORTYPE_BMP085
#define TGRAM_COMPONENT_BMP160	 REMOTE_SENSORTYPE_BMP160
#define TGRAM_COMPONENT_DS18B20	 REMOTE_SENSORTYPE_DS18B20
#define TGRAM_COMPONENT_DS1621	 REMOTE_SENSORTYPE_DS1621

#define TGRAM_COMPONENT_RGBLED	11
#define TGRAM_COMPONENT_2RELAIS	12

#define TGRAM_DATA_PAYLOAD_SIZE     10
#define TGRAM_DATA_TOTAL_SIZE	    12
#define TGRAM_PROT_PAYLOAD_SIZE	     9
#define TGRAM_PROT_TOTAL_SIZE	    11
#define TGRAM_CMD_PAYLOAD_SIZE	     9
#define TGRAM_CMD_TOTAL_SIZE	    11
#define TGRAM_RESULT_PAYLOAD_SIZE    9
#define TGRAM_RESULT_TOTAL_SIZE	    11
#define TGRAM_QUERY_PAYLOAD_SIZE     9
#define TGRAM_QUERY_TOTAL_SIZE	    11
#define TGRAM_INFO_PAYLOAD_SIZE	     9
#define TGRAM_INFO_TOTAL_SIZE	    11
#define TGRAM_REQUEST_PAYLOAD_SIZE   9
#define TGRAM_REQUEST_TOTAL_SIZE    11
#define TGRAM_RESPONSE_PAYLOAD_SIZE 10
#define TGRAM_RESPONSE_TOTAL_SIZE   12

#define TGRAM_MAX_SIZE		30


#define TGRAM_TYPE_DATA	                   1
#define TGRAM_TYPE_PROT	                   2
#define TGRAM_TYPE_CMD	                   3
#define TGRAM_TYPE_RESULT                  4
#define TGRAM_TYPE_QUERY                   5
#define TGRAM_TYPE_INFO	                   6
#define TGRAM_TYPE_REQUEST                 7
#define TGRAM_TYPE_RESPONSE                8

#define TGRAM_TPOS_MAGIC                    0
#define TGRAM_TPOS_PAYLOAD_SIZE             1
#define TGRAM_TPOS_TGRAM_TYPE               2
// DATA telegram
#define TGRAM_DATA_TPOS_STATION_ID          3
#define TGRAM_DATA_TPOS_SENSOR_ID           4
#define TGRAM_DATA_TPOS_SENSOR_TYPE         5
#define TGRAM_DATA_TPOS_TARGETSYSTEM_ID     6
#define TGRAM_DATA_TPOS_UNIT                7
#define TGRAM_DATA_TPOS_DATA_TYPE           8
// REQUEST telegram
#define TGRAM_REQUEST_TPOS_REQUEST_ID       3
#define TGRAM_REQUEST_TPOS_STATION_ID       4
// RESPONSE telegram
#define TGRAM_RESPONSE_TPOS_RESPONSE_TYPE   3
#define TGRAM_RESPONSE_TPOS_STATION_ID      4
#define TGRAM_RESPONSE_TPOS_RESPONSE_STATUS 5
#define TGRAM_RESPONSE_TPOS_TARGETSYSTEM_ID 6
#define TGRAM_RESPONSE_TPOS_DATA_SIZE       7
#define TGRAM_RESPONSE_TPOS_RESPONSE_DATA   8
//
// from this offset buffer contains response data
//
// RESPONSE for a sensor request
#define TGRAM_RESPONSE_TPOS_SENSOR_TYPE     8
#define TGRAM_RESPONSE_TPOS_SENSOR_ID       9

// RESPONSE for DS18B20 of a sensor request
#define TGRAM_RESPONSE_TPOS_DS18B20_ADDR   10



// DATA TGRAM value format/position depends on data type
// bool - one byte only
#define TGRAM_DATA_BOOL_VALUE_POS           9
// char - one byte only
#define TGRAM_DATA_CHAR_VALUE_POS           9
// uchar - one byte only
#define TGRAM_DATA_UCHAR_VALUE_POS          9
// short - two bytes
#define TGRAM_DATA_SHORT_MSB_POS            9
#define TGRAM_DATA_SHORT_LSB_POS           10
// ushort - two bytes
#define TGRAM_DATA_USHORT_MSB_POS           9
#define TGRAM_DATA_USHORT_LSB_POS          10
// long - four bytes
#define TGRAM_DATA_LONG_HIGH_WORD_MSB_POS   9
#define TGRAM_DATA_LONG_HIGH_WORD_LSB_POS  10
#define TGRAM_DATA_LONG_LOW_WORD_MSB_POS   11
#define TGRAM_DATA_LONG_LOW_WORD_LSB_POS   12
// ulong - four bytes
#define TGRAM_DATA_ULONG_HIGH_WORD_MSB_POS  9
#define TGRAM_DATA_ULONG_HIGH_WORD_LSB_POS 10
#define TGRAM_DATA_ULONG_LOW_WORD_MSB_POS  11
#define TGRAM_DATA_ULONG_LOW_WORD_LSB_POS  12
// float - four bytes: 
//    two for the value, two for the decimals
//    means −32.768 up to 32.767 as value
//          0 to 65.535 as decimal value
#define TGRAM_DATA_FLOAT_MSB_VALUE_POS      9
#define TGRAM_DATA_FLOAT_LSB_VALUE_POS     10
#define TGRAM_DATA_FLOAT_MSB_DECIMAL_POS   11
#define TGRAM_DATA_FLOAT_LSB_DECIMAL_POS   12
// raw - unspecified byte buffer
#define TGRAM_DATA_RAW_LEN_POS             9
#define TGRAM_DATA_RAW_VALUE_POS          10


// PROT TGRAM - ACK, NAK, ... only
// CMD TGRAM - telegram containing a command
// RESULT TGRAM - answer to a command telegram
// QUERY TGRAM - telegram to query status ... 
// INFO TGRAM - answer to a query telegram
// REQUEST TGRAM - telegram to request some value
// RESPONSE TGRAM - answer to a request


#define PROTOCOL_REQUEST_UNKNOWN   54
#define PROTOCOL_REQUEST_NONE      74
#define PROTOCOL_REQUEST_SENSORS   60


#define PROTOCOL_RESPONSE_SENSORS  70
#define PROTOCOL_RESPONSE_MORE     72
#define PROTOCOL_RESPONSE_COMPLETE 74


#define PROTOCOL_SEND_DATA	   33
#define PROTOCOL_WAIT_REQUEST      22
#define PROTOCOL_WAIT_TIME         44
#define PROTOCOL_UNKNOWN_MODE      -1

#define PROTOCOL_REPLY_FLOAT   27
#define PROTOCOL_REPLY_INT     28
#define PROTOCOL_REPLY_LONG    29
#define PROTOCOL_REPLY_STRING  30
#define PROTOCOL_REPLY_CHAR    31
#define PROTOCOL_REPLY_BOOL    32
#define PROTOCOL_REPLY_UNKNOWN 33
#define PROTOCOL_REPLY_ERROR   34

#define PROTOCOL_QUERY_TEMP         25
#define PROTOCOL_QUERY_ALTITUDE     26
#define PROTOCOL_QUERY_PRESSURE     27
#define PROTOCOL_QUERY_SEA_PRESSURE 28
#define PROTOCOL_QUERY_RPM          29
#define PROTOCOL_QUERY_VOLTAGE      30
#define PROTOCOL_QUERY_CURRENT      31
#define PROTOCOL_QUERY_RESISTANCE   32
#define PROTOCOL_QUERY_LUMEN        33
#define PROTOCOL_QUERY_DISTANCE     34
#define PROTOCOL_QUERY_LOUDNES      35
#define PROTOCOL_QUERY_WEIGHT       36
#define PROTOCOL_QUERY_FLOW         37

//
// *****************************************************************
//

extern int protocolMakeDataTelegram( struct sTgramParam *pParam,
                                     union uTdata *pData,
                                     unsigned char telegram[] );

extern void protocolDataTelegramInfo( unsigned char telegram[] );

extern int protocolMakeRequestTelegram( unsigned char station_id, unsigned char requestId, unsigned char telegram[] );

extern int protocolIsResponseComplete( unsigned char telegram[] );

extern void protocolResponseTelegramInfo( unsigned char telegram[] );

extern int protocolMakeResponseTelegram( unsigned char station_id, unsigned char responseType, unsigned char responseStatus, unsigned char targetSystemId, unsigned char telegram[], unsigned char payLoad[], unsigned char payLoadSize  );

extern int protocolProcessResponseTelegram( unsigned char telegram[] );
extern int protocolProcessDataTelegram( unsigned char telegram[], void *sensorEntry );
extern int protocolDataTelegram2EMONCMS( unsigned char telegram[], void *pSensorEntry );
extern int protocolDataTelegram2SHC( unsigned char telegram[], void *pSensorEntry );

extern int protocolDataTelegramProcessRStation( int rStation, int* responseCompleted, volatile unsigned char remoteStation[] );

#ifdef __cplusplus
}
#endif


#endif // _TELEGRAM_H_


