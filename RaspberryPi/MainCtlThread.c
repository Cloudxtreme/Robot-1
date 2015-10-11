/* *******************************************************************
 *
 * MainCtlThread.c
 *
 * (C) 2014 Dreamshader (Dirk Schanz)
 *
 *  part of a multi-process server.
 *     stuff that will run as a child process
 *     
 *
 * gcc -c MainCtlThread.c
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
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <termios.h>

#include "MasterControl.h"
#include "MainCtlThread.h"

#ifndef UBUNTU
#include "pigpio.h"
#endif // NOT UBUNTU

#ifdef SUBCTL_2SONIC
#include "SubCtl_2Sonic.h"
#endif // SUBCTL_2SONIC

#ifdef SUBCTL_4SONIC
#include "SubCtl_4Sonic.h"
#endif // SUBCTL_4SONIC

#ifdef SUBCTL_2MOTOR
#include "SubCtl_2Motor.h"
#endif // SUBCTL_2MOTOR

#ifdef SUBCTL_GYRO
#include "SubCtl_Gyro.h"
#endif // SUBCTL_GYRO

#ifdef SUBCTL_TFT
#include "SubCtl_Tft.h"
#endif // SUBCTL_TFT



/*
 * *************************************************************
 * initialize the main ctrl thread
 * int mainCtlInit( srv_opts_t *options, p_iic_vars_t i2cVars )
 * *************************************************************
*/
int mainCtlInit( srv_opts_t *options, p_iic_vars_t i2cVars )
{
   int errcode = E_NOERR;

   if( options != NULL && i2cVars != NULL )
   {
#ifndef UBUNTU
      if (gpioInitialise() < 0)
      {
         errcode = E_FAIL;
      }
      else
      {
         options->mainctl_run = 1;
         i2cVars->pigpioInitialized = 1;
      }

#ifdef SUBCTL_2SONIC
         initialize_2sonic( options, i2cVars );
#endif // SUBCTL_2SONIC

#ifdef SUBCTL_4SONIC
   i2cVars->i2cHandle4Sonic = i2cOpen( I2C_BUS_NO, I2C_SLAVE_4SONIC, 0);
#endif // SUBCTL_4SONIC

#ifdef SUBCTL_2MOTOR
   i2cVars->i2cHandleMotor = i2cOpen( I2C_BUS_NO, I2C_SLAVE_MOTOR, 0);
#endif // SUBCTL_2MOTOR

#ifdef SUBCTL_GYRO
   i2cVars->i2cHandleGyro = i2cOpen( I2C_BUS_NO, I2C_SLAVE_GYRO, 0);
#endif // SUBCTL_GYRO

#ifdef SUBCTL_TFT
   i2cVars->i2cHandleTft = i2cOpen( I2C_BUS_NO, I2C_SLAVE_TFT, 0);
#endif // SUBCTL_TFT

#endif // NOT UBUNTU
   }
   else
   {
      errcode = E_NULLPTR;
   }


   return( errcode );
}


/*
 * *************************************************************
 * cleanup IIC connections
 * void cleanup_mainctl(srv_opts_t *options,p_iic_vars_t i2cVars)
 * *************************************************************
*/
void cleanup_mainctl( srv_opts_t *options, p_iic_vars_t i2cVars )
{

   if( options != NULL && i2cVars != NULL )
   {
#ifndef UBUNTU

#ifdef SUBCTL_2SONIC
      release_2sonic( options, i2cVars );
#endif // SUBCTL_2SONIC

#ifdef SUBCTL_4SONIC
      i2cClose(i2cVars->i2cHandle4Sonic);
#endif // SUBCTL_4SONIC

#ifdef SUBCTL_2MOTOR
      i2cClose(i2cVars->i2cHandleMotor);
#endif // SUBCTL_2MOTOR

#ifdef SUBCTL_GYRO
      i2cClose(i2cVars->i2cHandleGyro);
#endif // SUBCTL_GYRO

#ifdef SUBCTL_TFT
      i2cClose(i2cVars->i2cHandleTft);
#endif // SUBCTL_TFT

#ifndef UBUNTU
      gpioTerminate();
      i2cVars->pigpioInitialized = 0;
#endif // NOT UBUNTU

#endif // NOT UBUNTU
   }

}

/*
 * *************************************************************
 * stuff for the main control thread
 * void *run_main_thread(void *runOptions)
 * *************************************************************
*/
void *run_main_thread(void *runOptions)
{
   iic_vars_t i2cControl;
   sonic2param_t i2cParam;
   struct thread_info* pInfo;
   srv_opts_t *options;
   int counter;
   int errcode;

   if( (pInfo = (struct thread_info*) runOptions) != NULL &&
       (options = (srv_opts_t*) pInfo->options) != NULL )
   {

      i2cControl.pParam2sonic = (void*) &i2cParam;
      mainCtlInit( runOptions, &i2cControl );
      options->MainThreadStatus = CTRL_THREAD_RUNNING;

      // main loop - never ends
      counter = 0;
      while( options->mainctl_run )
      {
         switch(++counter)
         {
            case 1:
               errcode = get2SonicCondition( runOptions, &i2cControl );
               sprintf( options->logbuffer, "errcode = %d [%s]\n", errcode, i2cControl.i2cInBuf );
               do_log( options, LOG_DBG );
               break;
            case 2:
               errcode = set2SonicCondRedDistance(runOptions, &i2cControl, 15);
               sprintf( options->logbuffer, "errcode = %d [%s]\n", errcode, i2cControl.i2cInBuf );
               do_log( options, LOG_DBG );
               break;
            case 3:
               errcode = set2SonicCondYellowDistance(runOptions, &i2cControl, 40);
               sprintf( options->logbuffer, "errcode = %d [%s]\n", errcode, i2cControl.i2cInBuf );
               do_log( options, LOG_DBG );
               break;
            case 4:
               errcode = set2SonicCondGreenDistance(runOptions, &i2cControl, 80);
               sprintf( options->logbuffer, "errcode = %d [%s]\n", errcode, i2cControl.i2cInBuf );
               do_log( options, LOG_DBG );
               break;
            case 5:
               errcode = get2SonicCondRedDistance(runOptions, &i2cControl );
               sprintf( options->logbuffer, "errcode = %d [%s]\n", errcode, i2cControl.i2cInBuf );
               do_log( options, LOG_DBG );
               break;
            case 6:
               errcode = get2SonicCondYellowDistance(runOptions, &i2cControl );
               sprintf( options->logbuffer, "errcode = %d [%s]\n", errcode, i2cControl.i2cInBuf );
               do_log( options, LOG_DBG );
               break;
            case 7:
               errcode = get2SonicCondGreenDistance(runOptions, &i2cControl );
               sprintf( options->logbuffer, "errcode = %d [%s]\n", errcode, i2cControl.i2cInBuf );
               do_log( options, LOG_DBG );
               break;
            case 8:
               errcode = get2SonicDirection( runOptions, &i2cControl );
               sprintf( options->logbuffer, "errcode = %d [%s]\n", errcode, i2cControl.i2cInBuf );
               do_log( options, LOG_DBG );
               break;
            case 9:
               errcode = get2SonicDistance( runOptions, &i2cControl );
               sprintf( options->logbuffer, "errcode = %d [%s]\n", errcode, i2cControl.i2cInBuf );
               do_log( options, LOG_DBG );
               break;
            case 10:
               errcode = set2SonicDirectiondForward( runOptions, &i2cControl );
               sprintf( options->logbuffer, "errcode = %d [%s]\n", errcode, i2cControl.i2cInBuf );
               do_log( options, LOG_DBG );
               break;
            case 11:
               errcode = set2SonicDirectiondBack( runOptions, &i2cControl );
               sprintf( options->logbuffer, "errcode = %d [%s]\n", errcode, i2cControl.i2cInBuf );
               do_log( options, LOG_DBG );
               break;
            case 12:
               errcode = set2SonicInterval( runOptions, &i2cControl, 358 );
               sprintf( options->logbuffer, "errcode = %d [%s]\n", errcode, i2cControl.i2cInBuf );
               do_log( options, LOG_DBG );
               break;
            case 13:
               errcode = get2SonicInterval( runOptions, &i2cControl );
               sprintf( options->logbuffer, "errcode = %d [%s]\n", errcode, i2cControl.i2cInBuf );
               do_log( options, LOG_DBG );
               break;
            case 14:
               errcode = set2SonicDirectiondRight( runOptions, &i2cControl );
               sprintf( options->logbuffer, "errcode = %d [%s]\n", errcode, i2cControl.i2cInBuf );
               do_log( options, LOG_DBG );
               break;
            case 15:
               errcode = set2SonicDirectiondLeft( runOptions, &i2cControl );
               sprintf( options->logbuffer, "errcode = %d [%s]\n", errcode, i2cControl.i2cInBuf );
               do_log( options, LOG_DBG );
               break;
            case 16:
               errcode = do2SonicUserCmd( runOptions, &i2cControl, (int) '0' );
               sprintf( options->logbuffer, "errcode = %d [%s]\n", errcode, i2cControl.i2cInBuf );
               do_log( options, LOG_DBG );
               break;
            case 17:
               errcode = do2SonicStop( runOptions, &i2cControl );
               sprintf( options->logbuffer, "errcode = %d [%s]\n", errcode, i2cControl.i2cInBuf );
               do_log( options, LOG_DBG );
               break;
            case 18:
               errcode = do2SonicPause( runOptions, &i2cControl );
               sprintf( options->logbuffer, "errcode = %d [%s]\n", errcode, i2cControl.i2cInBuf );
               do_log( options, LOG_DBG );
               break;
            case 19:
               errcode = do2SonicContinue( runOptions, &i2cControl );
               sprintf( options->logbuffer, "errcode = %d [%s]\n", errcode, i2cControl.i2cInBuf );
               do_log( options, LOG_DBG );
               break;
            case 20:
               errcode = do2SonicExit( runOptions, &i2cControl );
               sprintf( options->logbuffer, "errcode = %d [%s]\n", errcode, i2cControl.i2cInBuf );
               do_log( options, LOG_DBG );
               break;
            case 21:
               errcode = do2SonicStoreValues( runOptions, &i2cControl );
               sprintf( options->logbuffer, "errcode = %d [%s]\n", errcode, i2cControl.i2cInBuf );
               do_log( options, LOG_DBG );
#ifndef UBUNTU
         gpioSleep(PI_TIME_RELATIVE, 5, 000000); // sleep for 5 seconds
#endif // NOT UBUNTU
               break;
            case 22:
               errcode = set2SonicVerbose( runOptions, &i2cControl, 12 );
               sprintf( options->logbuffer, "errcode = %d [%s]\n", errcode, i2cControl.i2cInBuf );
               do_log( options, LOG_DBG );
               break;
            case 23:
               errcode = set2SonicDirection( runOptions, &i2cControl, (int) 'f' );
               sprintf( options->logbuffer, "errcode = %d [%s]\n", errcode, i2cControl.i2cInBuf );
               do_log( options, LOG_DBG );
               break;
            case 24:
               errcode = set2SonicDistance( runOptions, &i2cControl, 18 );
               sprintf( options->logbuffer, "errcode = %d [%s]\n", errcode, i2cControl.i2cInBuf );
               do_log( options, LOG_DBG );
               break;
            case 25:
               errcode = set2SonicCondition( runOptions, &i2cControl, (int) 'r' );
               sprintf( options->logbuffer, "errcode = %d [%s]\n", errcode, i2cControl.i2cInBuf );
               do_log( options, LOG_DBG );
               break;
            case 26:
               errcode = set2SonicDebug( runOptions, &i2cControl, (int) 'g' );
               sprintf( options->logbuffer, "errcode = %d [%s]\n", errcode, i2cControl.i2cInBuf );
               do_log( options, LOG_DBG );
               break;
            case 27:
               errcode = set2SonicDefaults( runOptions, &i2cControl );
               sprintf( options->logbuffer, "errcode = %d [%s]\n", errcode, i2cControl.i2cInBuf );
               do_log( options, LOG_DBG );
               break;
            case 28:
               errcode = get2SonicMaxAlarm( runOptions, &i2cControl );
               sprintf( options->logbuffer, "errcode = %d [%s]\n", errcode, i2cControl.i2cInBuf );
               do_log( options, LOG_DBG );
               break;
            case 29:
               errcode = set2SonicMaxAlarm( runOptions, &i2cControl, 5 );
               sprintf( options->logbuffer, "errcode = %d [%s]\n", errcode, i2cControl.i2cInBuf );
               do_log( options, LOG_DBG );
               counter = 0;
               break;
         }
#ifndef UBUNTU
         gpioSleep(PI_TIME_RELATIVE, 0, 500000); // sleep for 0.3 seconds
#endif // NOT UBUNTU
      }

      cleanup_mainctl( runOptions, &i2cControl );
   }

   return(NULL);
}


