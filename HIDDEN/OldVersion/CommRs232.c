/* *******************************************************************
 *
 * CommRs232.c
 *
 * (C) 2014 Dreamshader (Dirk Schanz)
 *
 *  part of a multi-process server.
 *     Stuff for handling a rs232 connection.
 *     
 *
 * gcc -c CommRs232.c
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

#include "CommRs232.h"

volatile int rs232_data;

/*
 * *************************************************************
 * typedef void (*sighandler_t)(int);
 * Declaration for a signal handler
 * *************************************************************
*/
typedef void (*sighandler_t)(int);

/*
 * *************************************************************
 * static sighandler_t handle_signal (int sig_nr, sighandler_t signalhandler) 
 * setup a new signal handling for a specific signal
 * *************************************************************
*/
static sighandler_t handle_signal (int sig_nr, sighandler_t signalhandler) 
{
   struct sigaction neu_sig, alt_sig;
   neu_sig.sa_handler = signalhandler;
   sigemptyset (&neu_sig.sa_mask);
   neu_sig.sa_flags = SA_RESTART;
   if (sigaction (sig_nr, &neu_sig, &alt_sig) < 0)
      return SIG_ERR;
   return alt_sig.sa_handler;
}

/*
 * *************************************************************
 * static void catch_SIGIO(int val)
 * A simple handler that can be installed using handle_signal
 * *************************************************************
*/
static void catch_SIGIO(int val)
{
   rs232_data = 1;
   return;
}

/*
 * *************************************************************
 * int rs232Open( struct serial_param_t *ctl_param )
 * open serial device
 * *************************************************************
*/
int rs232Open( struct serial_param_t *ctl_param )
{
   if( ctl_param != NULL )
   {
      memset(ctl_param->logbuffer, '\0', sizeof(ctl_param->logbuffer) );
      ctl_param->dev_fd = open( ctl_param->device_name,
                                O_RDWR|O_NOCTTY|O_NONBLOCK);
      return( ctl_param->dev_fd );
   }

   return( E_RS232_FAIL );
}

/*
 * *************************************************************
 * void rs232Close( struct serial_param_t *ctl_param )
 * close the serial device
 * *************************************************************
*/
void rs232Close( struct serial_param_t *ctl_param )
{
   if( ctl_param != NULL )
   {
      memset(ctl_param->logbuffer, '\0', sizeof(ctl_param->logbuffer) );
      if( ctl_param->dev_fd > 0 )
      {
         close( ctl_param->dev_fd );
         ctl_param->dev_fd = 0;
      }
   }
}

/*
 * *************************************************************
 * int rs232Reset( struct serial_param_t *ctl_param )
 * reset old parameter of serial port
 * *************************************************************
*/
int rs232Reset( struct serial_param_t *ctl_param )
{
   int fail;

   if( ctl_param != NULL )
   {
      memset(ctl_param->logbuffer, '\0', sizeof(ctl_param->logbuffer) );
      fail = tcsetattr( ctl_param->dev_fd, TCSANOW, &ctl_param->oldsettings );
   }
   else
   {
      fail = E_RS232_FAIL;
   }

   return(fail);
}

/*
 * *************************************************************
 * int rs232GetParam( struct serial_param_t *ctl_param,
 *                    struct termios *mytio )
 * get current serial parameters
 * *************************************************************
*/
int rs232GetParam( struct serial_param_t *ctl_param, struct termios *mytio )
{
   int fail = 0;

   if( ctl_param != NULL && mytio != NULL )
   {
      memset(ctl_param->logbuffer, '\0', sizeof(ctl_param->logbuffer) );
      if( ctl_param->dev_fd > 0 )
      {
         fail = tcgetattr( ctl_param->dev_fd, mytio );
      }
      else
      {
         fail = E_RS232_FAIL;
         sprintf(&ctl_param->logbuffer[strlen(ctl_param->logbuffer)],
            "wrong fd %d\n", ctl_param->dev_fd );
      }
   }
   else
   {
      fail = E_RS232_FAIL;
   }
   return( fail );
}

/*
 * *************************************************************
 * int rs232SetUpRaw( struct serial_param_t *ctl_param,
 *                    struct termios *mytio )
 * set parameter for raw communication
 * *************************************************************
*/
int rs232SetUpRaw( struct serial_param_t *ctl_param, struct termios *mytio )
{
   int fail = 0;

   if( ctl_param != NULL && mytio != NULL )
   {
      memset(ctl_param->logbuffer, '\0', sizeof(ctl_param->logbuffer) );
      if( ctl_param->dev_fd > 0 )
      {
         cfmakeraw( mytio );
      }
      else
      {
         fail = E_RS232_FAIL;
         sprintf(&ctl_param->logbuffer[strlen(ctl_param->logbuffer)],
            "wrong fd %d\n", ctl_param->dev_fd );
      }
   }
   else
   {
      fail = E_RS232_FAIL;
   }
   return( fail );
}

/*
 * *************************************************************
 * int rs232SetUpParam( struct serial_param_t *ctl_param,
 *                    struct termios *mytio )
 * set parameter for serial communication
 * *************************************************************
*/
int rs232SetUpParam( struct serial_param_t *ctl_param, struct termios *mytio )
{
   int fail = 0;

   if( ctl_param != NULL && mytio != NULL )
   {
      memset(ctl_param->logbuffer, '\0', sizeof(ctl_param->logbuffer) );
      if( ctl_param->dev_fd > 0 )
      {
         cfmakeraw( mytio );
         if( !(fail = rs232SetUpBaudrate( ctl_param, mytio )) )
         {
            if( !(fail = rs232SetUpDatabits( ctl_param, mytio )) )
            {
               if( !(fail = rs232SetUpParity( ctl_param, mytio )) )
               {
                  if( !(fail = rs232SetUpStoppbits( ctl_param, mytio )) )
                  {
                     fail = rs232SetUpHandshake( ctl_param, mytio );
                  }
               }
            }
         }
      }
      else
      {
         fail = E_RS232_FAIL;
         sprintf(&ctl_param->logbuffer[strlen(ctl_param->logbuffer)],
            "wrong fd %d\n", ctl_param->dev_fd );
      }
   }
   else
   {
      fail = E_RS232_FAIL;
   }
   return( fail );
}

/*
 * *************************************************************
 * int rs232SetUpBaudrate( struct serial_param_t *ctl_param,
 *                         struct termios *mytio )
 * set desired baudrate in parameters
 * *************************************************************
*/
int rs232SetUpBaudrate( struct serial_param_t *ctl_param, struct termios *mytio )
{
   int fail = 0;

   if( ctl_param != NULL && mytio != NULL )
   {
      if( ctl_param->dev_fd > 0 )
      {
 
         switch( ctl_param->baud )
         {
            case     50:
               cfsetspeed ( mytio, B50 );
               sprintf(&ctl_param->logbuffer[strlen(ctl_param->logbuffer)],
                  "B50\n" );
               break;
            case     75:
               cfsetspeed ( mytio, B75 );
               sprintf(&ctl_param->logbuffer[strlen(ctl_param->logbuffer)],
                  "B75\n" );
               break;
            case    110:
               cfsetspeed ( mytio, B110 );
               sprintf(&ctl_param->logbuffer[strlen(ctl_param->logbuffer)],
                  "B110\n" );
               break;
            case    134:
               cfsetspeed ( mytio, B134 );
               sprintf(&ctl_param->logbuffer[strlen(ctl_param->logbuffer)],
                  "B134\n" );
               break;
            case    150:
               cfsetspeed ( mytio, B150 );
               sprintf(&ctl_param->logbuffer[strlen(ctl_param->logbuffer)],
                  "B150\n" );
               break;
            case    200:
               cfsetspeed ( mytio, B200 );
               sprintf(&ctl_param->logbuffer[strlen(ctl_param->logbuffer)],
                  "B200\n" );
               break;
            case    300:
               cfsetspeed ( mytio, B300 );
               sprintf(&ctl_param->logbuffer[strlen(ctl_param->logbuffer)],
                  "B300\n" );
               break;
            case    600:
               cfsetspeed ( mytio, B600 );
               sprintf(&ctl_param->logbuffer[strlen(ctl_param->logbuffer)],
                  "B600\n" );
               break;
            case   1200:
               cfsetspeed ( mytio, B1200 );
               sprintf(&ctl_param->logbuffer[strlen(ctl_param->logbuffer)],
                  "B1200\n" );
               break;
            case   1800:
               cfsetspeed ( mytio, B1800 );
               sprintf(&ctl_param->logbuffer[strlen(ctl_param->logbuffer)],
                  "B1800\n" );
               break;
            case   2400:
               cfsetspeed ( mytio, B2400 );
               sprintf(&ctl_param->logbuffer[strlen(ctl_param->logbuffer)],
                  "B2400\n" );
               break;
            case   4800:
               cfsetspeed ( mytio, B4800 );
               sprintf(&ctl_param->logbuffer[strlen(ctl_param->logbuffer)],
                  "B4800\n" );
               break;
            case   9600:
               cfsetspeed ( mytio, B9600 );
               sprintf(&ctl_param->logbuffer[strlen(ctl_param->logbuffer)],
                  "B9600\n" );
               break;
            case  19200:
               cfsetspeed ( mytio, B19200 );
               sprintf(&ctl_param->logbuffer[strlen(ctl_param->logbuffer)],
                  "B19200\n" );
               break;
            case  38400:
               cfsetspeed ( mytio, B38400 );
               sprintf(&ctl_param->logbuffer[strlen(ctl_param->logbuffer)],
                  "B3840\n" );
               break;
            case  57600:
               cfsetspeed ( mytio, B57600 );
               sprintf(&ctl_param->logbuffer[strlen(ctl_param->logbuffer)],
                  "B57600\n" );
               break;
            case 115200:
               cfsetspeed ( mytio, B115200 );
               sprintf(&ctl_param->logbuffer[strlen(ctl_param->logbuffer)],
                  "B115200\n" );
               break;
            case 230400:
               cfsetspeed ( mytio, B230400 );
               sprintf(&ctl_param->logbuffer[strlen(ctl_param->logbuffer)],
                  "B230400\n" );
               break;
            case 460800:
               sprintf(&ctl_param->logbuffer[strlen(ctl_param->logbuffer)],
                  "B460800\n" );
               cfsetspeed ( mytio, B460800 );
               break;
            default:
               fail = E_RS232_FAIL;
               sprintf(&ctl_param->logbuffer[strlen(ctl_param->logbuffer)],
                  "BD-ERROR\n" );
         }
      }
      else
      {
         fail = E_RS232_FAIL;
         sprintf(&ctl_param->logbuffer[strlen(ctl_param->logbuffer)],
            "wrong fd %d\n", ctl_param->dev_fd );
      }
   }
   else
   {
      fail = E_RS232_FAIL;
   }
   return( fail );
}

/*
 * *************************************************************
 * int rs232SetUpDatabits( struct serial_param_t *ctl_param,
 *                         struct termios *mytio )
 * set desired databits in parameters
 * *************************************************************
*/
int rs232SetUpDatabits( struct serial_param_t *ctl_param, struct termios *mytio )
{
   int fail = 0;

   if( ctl_param != NULL && mytio != NULL )
   {
      if( ctl_param->dev_fd > 0 )
      {
 
         switch( ctl_param->databit )
         {
            case 5:
               mytio->c_cflag |= CS5;
               sprintf(&ctl_param->logbuffer[strlen(ctl_param->logbuffer)],
                  "DB5\n" );
               break;
            case 6:
               mytio->c_cflag |= CS6;
               sprintf(&ctl_param->logbuffer[strlen(ctl_param->logbuffer)],
                  "DB6\n" );
               break;
            case 7:
               mytio->c_cflag |= CS7;
               sprintf(&ctl_param->logbuffer[strlen(ctl_param->logbuffer)],
                  "DB7\n" );
               break;
            case 8:
               mytio->c_cflag |= CS8;
               sprintf(&ctl_param->logbuffer[strlen(ctl_param->logbuffer)],
                  "DB8\n" );
               break;
            default:
               fail = E_RS232_FAIL;
               sprintf(&ctl_param->logbuffer[strlen(ctl_param->logbuffer)],
                  "DB-ERROR\n" );
         }
      }
      else
      {
         fail = E_RS232_FAIL;
         sprintf(&ctl_param->logbuffer[strlen(ctl_param->logbuffer)],
            "wrong fd %d\n", ctl_param->dev_fd );
      }
   }
   else
   {
      fail = E_RS232_FAIL;
   }
   return( fail );
}

/*
 * *************************************************************
 * int rs232SetUpParity( struct serial_param_t *ctl_param,
 *                       struct termios *mytio )
 * set desired parity in parameters
 * *************************************************************
*/
int rs232SetUpParity( struct serial_param_t *ctl_param, struct termios *mytio )
{
   int fail = 0;

   if( ctl_param != NULL && mytio != NULL )
   {
      if( ctl_param->dev_fd > 0 )
      {
   
         switch( ctl_param->parity )
         {
            case 'o':
            case 'O':
               mytio->c_cflag |= PARODD | PARENB;
               sprintf(&ctl_param->logbuffer[strlen(ctl_param->logbuffer)],
                  "PAR-O\n" );
               break;
            case 'e':
            case 'E':
               mytio->c_cflag |= PARENB;
               sprintf(&ctl_param->logbuffer[strlen(ctl_param->logbuffer)],
                  "PAR-E\n" );
               break;
            case 'n':
            case 'N':
               mytio->c_cflag |= IGNPAR;
               mytio->c_cflag &= ~PARENB;
               sprintf(&ctl_param->logbuffer[strlen(ctl_param->logbuffer)],
                  "PAR-N\n" );
               break;
            default:
               fail = E_RS232_FAIL;
               sprintf(&ctl_param->logbuffer[strlen(ctl_param->logbuffer)],
                  "PAR-ERROR\n" );
         }
      }
      else
      {
         fail = E_RS232_FAIL;
         sprintf(&ctl_param->logbuffer[strlen(ctl_param->logbuffer)],
            "wrong fd %d\n", ctl_param->dev_fd );
      }
   }
   else
   {
      fail = E_RS232_FAIL;
   }
   return( fail );
}

/*
 * *************************************************************
 * int rs232SetUpStoppbits( struct serial_param_t *ctl_param,
 *                          struct termios *mytio )
 * set desired stopbits in parameters
 * *************************************************************
*/
int rs232SetUpStoppbits( struct serial_param_t *ctl_param, struct termios *mytio )
{
   int fail = 0;

   if( ctl_param != NULL && mytio != NULL )
   {
      if( ctl_param->dev_fd > 0 )
      {
 
         switch( ctl_param->stoppbits )
         {
            case 1:
               sprintf(&ctl_param->logbuffer[strlen(ctl_param->logbuffer)],
                  "STB-1\n" );
               break;
            case 2:
               mytio->c_cflag |= CSTOPB;
               sprintf(&ctl_param->logbuffer[strlen(ctl_param->logbuffer)],
                  "STB-2\n" );
               break;
            default:
               fail = E_RS232_FAIL;
               sprintf(&ctl_param->logbuffer[strlen(ctl_param->logbuffer)],
                  "STB-ERROR\n" );
         }
      }
      else
      {
         fail = E_RS232_FAIL;
         sprintf(&ctl_param->logbuffer[strlen(ctl_param->logbuffer)],
            "wrong fd %d\n", ctl_param->dev_fd );
      }
   }
   else
   {
      fail = E_RS232_FAIL;
   }
   return( fail );
}

/*
 * *************************************************************
 * int rs232SetUpHandshake( struct serial_param_t *ctl_param,
 *                          struct termios *mytio )
 * set desired handshake in parameters
 * *************************************************************
*/
int rs232SetUpHandshake( struct serial_param_t *ctl_param, struct termios *mytio )
{
   int fail = 0;

   if( ctl_param != NULL && mytio != NULL )
   {
      if( ctl_param->dev_fd > 0 )
      {
         switch( ctl_param->handshake )
         {
            case 'n':
            case 'N':
               mytio->c_cflag |= CLOCAL;
               sprintf(&ctl_param->logbuffer[strlen(ctl_param->logbuffer)],
                  "HSK-N\n" );
               break;
            case 'x':
            case 'X':
               mytio->c_iflag |= IXON;
               sprintf(&ctl_param->logbuffer[strlen(ctl_param->logbuffer)],
                  "HSK-X\n" );
               break;
            default:
               fail = E_RS232_FAIL;
               sprintf(&ctl_param->logbuffer[strlen(ctl_param->logbuffer)],
                  "HSK-ERROR\n" );
         }
      }
      else
      {
         fail = E_RS232_FAIL;
         sprintf(&ctl_param->logbuffer[strlen(ctl_param->logbuffer)],
            "wrong fd %d\n", ctl_param->dev_fd );
      }
   }
   else
   {
      fail = E_RS232_FAIL;
   }
   return( fail );
}


/*
 * *************************************************************
 * int rs232CheckParam( struct serial_param_t *ctl_param, 
 *                 struct termios *newtio,
 *                 struct termios *savtio )
 * finally set desired serial parameters and check them
 * *************************************************************
*/
int rs232CheckParam( struct serial_param_t *ctl_param, 
                struct termios *newtio, struct termios *savtio, int *ok )
{
   int IsOk = 0;
   int fail = 0;

   if( ctl_param != NULL )
   {
      if( newtio != NULL && savtio != NULL && ok != NULL )
      {
         if( ctl_param->dev_fd > 0 )
         {
            if( !(fail = tcgetattr( ctl_param->dev_fd, newtio )) )
            {

               if( cfgetispeed( newtio ) == cfgetispeed( savtio ) &&
                   cfgetospeed( newtio ) == cfgetospeed( savtio ) )
               {
                  IsOk = 1;
                  *ok = IsOk;
               }
               else
               {
                  rs232SetUpBaudrate( ctl_param, newtio );
               }

               if( newtio->c_cflag != savtio->c_cflag &&
                   newtio->c_iflag != savtio->c_iflag )
               {
                  IsOk = 0;
                  *ok = IsOk;
                  rs232SetUpDatabits( ctl_param, newtio );
                  rs232SetUpParity( ctl_param, newtio );
                  rs232SetUpStoppbits( ctl_param, newtio );
                  rs232SetUpHandshake( ctl_param, newtio );
               }
            }
            else
            {
               fail = E_RS232_FAIL;
               sprintf(&ctl_param->logbuffer[strlen(ctl_param->logbuffer)],
                  "failed get param %d, %s\n", errno, strerror(errno) );
            }

         }
         else
         {
            fail = E_RS232_FAIL;
            sprintf(&ctl_param->logbuffer[strlen(ctl_param->logbuffer)],
               "wrong fd %d\n", ctl_param->dev_fd );
         }
      }
      else
      {
         fail = E_RS232_FAIL;
         sprintf(&ctl_param->logbuffer[strlen(ctl_param->logbuffer)],
            "NULLP\n" );
      }
   }
   else
   {
      fail = E_RS232_FAIL;
   }
   
   return( fail );
}

/*
 * *************************************************************
 * int rs232SetUp( struct serial_param_t *ctl_param, 
 *                 struct termios *newtio,
 *                 struct termios *savtio,
 *                 int *retries )
 * finally set desired serial parameters and check them
 * *************************************************************
*/
int rs232SetUp( struct serial_param_t *ctl_param, 
                struct termios *newtio, struct termios *savtio, int *retries )
{
   int fail = 0;
   int triesDone = 0;
   int checkOk = 0;

   if( ctl_param != NULL )
   {
      if( newtio != NULL && savtio != NULL && retries != NULL )
      {
         if( ctl_param->dev_fd > 0 )
         {
            checkOk = 0;
            for( triesDone = 0; checkOk == 0 && triesDone < *retries ; triesDone++ )
            {
               tcsetattr( ctl_param->dev_fd, TCSANOW, newtio );
               sprintf(&ctl_param->logbuffer[strlen(ctl_param->logbuffer)],
                  "setattr %d, %s\n", ctl_param->dev_fd, strerror(errno) );
               fail = rs232CheckParam( ctl_param, newtio, savtio, &checkOk );
            }
         }
         else
         {
            fail = E_RS232_FAIL;
            sprintf(&ctl_param->logbuffer[strlen(ctl_param->logbuffer)],
               "wrong fd %d\n", ctl_param->dev_fd );
         }
      }
      else
      {
         fail = E_RS232_FAIL;
         sprintf(&ctl_param->logbuffer[strlen(ctl_param->logbuffer)],
            "NULLP\n" );
      }
   }
   else
   {
      fail = E_RS232_FAIL;
   }
   
   return( fail );
}

