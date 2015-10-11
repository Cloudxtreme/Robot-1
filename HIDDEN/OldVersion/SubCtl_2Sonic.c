/* *******************************************************************
 *
 * SubCtl_2Sonic.c
 *
 * (C) 2014 Dreamshader (Dirk Schanz)
 *
 * control of an ultrasonic subcontroller (pro mini with two us-sensors)
 *     code part
 *
 * -------------------------------------------------------------------
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * *******************************************************************
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <getopt.h>
#include <syslog.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <sys/signal.h>
#include <sys/time.h>
#include <sys/types.h> 
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <termios.h>


#ifndef UBUNTU
#include "pigpio.h"
#endif // NOT UBUNTU

#include "CommRs232.h"
#include "AllControl.h"
#include "SubCtl_2Sonic.h"

/*
 * *************************************************************
 * global flag ...
 * *************************************************************
*/
int Sub2SonicCritical = 0;

#ifndef UBUNTU
/*
 * *************************************************************
 * handler for critical condition -> set flag
 * *************************************************************
*/
void alert2Sonic(int gpio, int level, uint32_t tick ) //void* udata
{
  Sub2SonicCritical = 1;
//    printf("gpio %d became %d at %d\n", gpio, level, tick);
}

/*
 * *************************************************************
 * initialize two sonic module
 * *************************************************************
*/
int initialize_2sonic( srv_opts_t *options )
{
   int errcode = E_NOERR;

   if( options != NULL )
   {
      gpioSetMode(PIN_ALARM_2SONIC, PI_INPUT);
//      gpioSetPullUpDown(PIN_ALARM_2SONIC, PI_PUD_UP);
      gpioSetAlertFunc(PIN_ALARM_2SONIC, alert2Sonic);
//     gpioSetAlertFuncEx(PIN_ALARM_2SONIC, alertSonic, void* userdata);
   }

   return( errcode );
}

/*
 * *************************************************************
 * int check2SonicCondition( srv_opts_t *options )
 * check for current sonic condition
 * *************************************************************
*/

int check2SonicCondition( srv_opts_t *options )
{
   int condition = CONDITION_2SONIC_G;
   char *ptr;
   int rc;

// #define CONDITION_2SONIC_R	3
// #define CONDITION_2SONIC_Y	4
// #define CONDITION_2SONIC_G	5

   if( Sub2SonicCritical )
   {
      // fprintf(stderr, "CRITICAL CONDITION!\n");
      Sub2SonicCritical = 0;
      condition = CONDITION_2SONIC_R;
   }
   else
   {
      sprintf(options->i2cOutBuf, "c#");
      rc = i2cWriteDevice(options->i2cHandle2Sonic, options->i2cOutBuf,
                          strlen(options->i2cOutBuf) );
      memset(options->i2cInBuf, '\0', sizeof(options->i2cInBuf));
      rc = i2cReadDevice(options->i2cHandle2Sonic, options->i2cInBuf,
                         I2C_MSG_LEN);
      if( ptr = strchr(options->i2cInBuf, 0x0ff) )
      {
         *ptr = '\0';
      }
   }

   return(condition);
}
#endif // NOT UBUNTU


