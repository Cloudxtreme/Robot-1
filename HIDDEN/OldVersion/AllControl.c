/* *******************************************************************
 *
 * AllControl.c
 *
 * (C) 2014 Dreamshader (Dirk Schanz)
 *
 * prototype for a multi process tcp/ip server
 *     runs as a daemon in the background and waiting for incoming
 *     connections. The client then may send some initial parameters
 *     that are parsed by the server. If all is ok, a child process
 *     is forked, taking the current connection for furthermore 
 *     processing.
 *     goal is to multiplex some serial connections to several clients
 *     where only one client may have read-write access. All other
 *     clients are allowed to read only the serial connection.
 *     Child process stuff handling the rs232 is not part of this 
 *     source file but included in CommRs232.c
 *
 * gcc -o AllControl AllControl.c CommRs232.c -lrt
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

#include "CommRs232.h"
#include "AllControl.h"
#include "CommProc.h"

#ifndef UBUNTU
#include "pigpio.h"
#endif // NOT UBUNTU

#ifdef SUBCTL_2SONIC
#include "SubCtl_2Sonic.h"
#endif // SUBCTL_2SONIC

#ifdef SUBCTL_4SONIC
#include "SubCtl_4Sonic.h"
#endif // SUBCTL_4SONIC

#ifdef SUBCTL_MOTOR
#include "SubCtl_Motor.h"
#endif // SUBCTL_MOTOR

#ifdef SUBCTL_GYRO
#include "SubCtl_Gyro.h"
#endif // SUBCTL_GYRO

#ifdef SUBCTL_TFT
#include "SubCtl_Tft.h"
#endif // SUBCTL_TFT

// misc stuff
char *version = "0.90a";

/*
 * *************************************************************
 * close log file(s)
 * *************************************************************
*/
void closelogfile( srv_opts_t *options )
{
   return;
}

/*
 * *************************************************************
 * Log function
 * *************************************************************
*/
void do_log( srv_opts_t *options, int level )
{
   char buff[100];
   time_t now;

   if( level <= options->verbose_level )
   {
      if( options->run_mode == RUN_FOREGROUND )
      {
         now = time(NULL);
         strftime (buff, 100, "%b %d %H:%M:%S", localtime (&now));
         fprintf(stderr, "%s  %s[%d]: %s", buff, options->daemon_name, 
            getpid(), options->logbuffer);
      }

      if( options->run_mode == RUN_DAEMON )
      {
         switch(level)
         {
            case LOG_FATL:
               syslog( LOG_EMERG, "%s", options->logbuffer );
               break;
            case LOG_CRITERROR:
               syslog( LOG_ALERT, "%s", options->logbuffer );
               break;
            case LOG_ERROR:
               syslog( LOG_ERR, "%s", options->logbuffer );
               break;
            case LOG_CRITWARN:
               syslog( LOG_CRIT, "%s", options->logbuffer );
               break;
            case LOG_WARN:
               syslog( LOG_WARNING, "%s", options->logbuffer );
               break;
            case LOG_INF_0:
               syslog( LOG_NOTICE, "%s", options->logbuffer );
               break;
            case LOG_INF_1:
            case LOG_INF_2:
               syslog( LOG_INFO, "%s", options->logbuffer );
               break;
            case LOG_DBG:
               syslog( LOG_DEBUG, "%s", options->logbuffer );
               break;
         }
      }
   }
}

/*
 * *************************************************************
 * handler for error and warning conditions
 * *************************************************************
*/
void ErrHndl( srv_opts_t *options, int errCode )
{
    switch( errCode )
    {
        case E_F_GETFL:
            fprintf(stderr, "fcntl(F_GETFL) failed\n");
            break;
        case E_F_SETFL:
            fprintf(stderr, "fcntl(F_SETFL) failed\n");
            break;
    }
}

/*
 * *************************************************************
 * Suspend daemon with given exit code
 * *************************************************************
*/
void suspend( srv_opts_t *options )
{
   if( options != NULL )
   {
      clock_gettime( CLOCK_REALTIME, &options->end_time );
      closelogfile(options);
      unlink( options->pid_file_name );
      exit( options->error_code);
   }
   else
   {
      exit( 0 );
   }
}

/*
 * *************************************************************
 * initialize runtime vars with defaults
 * *************************************************************
*/
void initialize( srv_opts_t *options )
{
   if( options != NULL )
   {

#ifndef UBUNTU
      if (gpioInitialise() < 0)
      {
         fprintf(stderr, "failed to initialize pigpio!\n");
         exit(1);
      }

      options->pigpioInitialized = 1;

      options->i2cHandle2Sonic = i2cOpen( I2C_BUS_NO, I2C_SLAVE_2SONIC, 0);

//  rc = i2cReadDevice(i2cHandleSonic, i2cInBuf, I2C_MSG_LEN);

#endif // NOT UBUNTU

      options->daemon_name = DEFAULT_DAEMONNAME;
      options->run_mode = DEFAULT_RUNMODE;
      options->error_code = E_NOERR;
      options->sys_errno = 0;
      options->verbose_level = DEFAULT_VERBOSE;
      options->debug_level = DEFAULT_DEBUG;
      options->run_as_daemon = DEFAULT_DAEMON;
      options->listen_port = DEFAULT_PORT;
      options->backlog = DEFAULT_BACKLOG;

      clock_gettime( CLOCK_REALTIME, &options->start_time );
      clock_gettime( CLOCK_REALTIME, &options->end_time );

      options->log_facility = LOG_LOCAL0;
      options->log_options = LOG_PID | LOG_CONS| LOG_NDELAY;
      options->pid_file_name = PID_FILE;

      options->p_log = &options->logbuffer[0];

      options->socket = -1;
      options->cli_socket = -1;
      memset(&options->local_addr, 0, sizeof(options->local_addr));
      memset(&options->remote_addr, 0, sizeof(options->remote_addr));
      options->local_addr.sin_port = htons(DEFAULT_PORT);
      options->sock_len = sizeof(options->remote_addr);
      options->local_addr.sin_family = AF_INET;
      options->local_addr.sin_addr.s_addr = INADDR_ANY;

      options->p_rcv_buf = &options->rcv_buf[0];
      memset(options->p_rcv_buf, 0, sizeof(options->rcv_buf));
      options->p_rsp_buf = &options->rctl_rsp_buf[0];
      memset(options->p_rsp_buf, 0, sizeof(options->rctl_rsp_buf));

      options->serial_dev.dev_fd = INIT_RS232_FD;
      options->serial_dev.device_name = NULL;
      options->serial_dev.baud = DEFAULT_RS232_BAUD;
      options->serial_dev.parity = DEFAULT_RS232_PARITY;
      options->serial_dev.databit = DEFAULT_RS232_DATAB;
      options->serial_dev.stoppbits = DEFAULT_RS232_STOPB;
      options->serial_dev.handshake = DEFAULT_RS232_HANDSHAKE;
      options->serial_dev.async = 0;
      options->serial_dev.dev_mode = NULL;

      options->child_entry = 0;
      options->childs = (clienttab_t**) NULL;
      if( ( options->childs = realloc( options->childs, 
            (options->child_entry+1 * sizeof(clienttab_t*)) )) != NULL )
      {
         options->childs[options->child_entry] = NULL;
      }

   }
}

/*
 * *************************************************************
 * create a server socket
 * *************************************************************
*/
static int make_socket( srv_opts_t *options )
{
   int on = 1;
   int error_code = E_NOERR;

   if((options->socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
   {
      options->sys_errno = errno;
      error_code = E_FAIL;
   }
   else
   {
      if(setsockopt(options->socket, SOL_SOCKET, SO_REUSEADDR,
         (char *)&on, sizeof(on)) < 0)
      {
         close(options->socket);
         options->sys_errno = errno;
         error_code = E_FAIL;
      }
      else
      {
         options->local_addr.sin_family = AF_INET;
         options->local_addr.sin_addr.s_addr = INADDR_ANY;
         if(bind(options->socket, (struct sockaddr*) &options->local_addr, 
            sizeof(options->local_addr)) < 0)
         {
            close(options->socket);
            options->sys_errno = errno;
            error_code = E_FAIL;
         }
      }
   }
   return (error_code);
}


/*
 * *************************************************************
 * set fd to non blocking
 * *************************************************************
*/
int nonblock(int fd)
{
    int opts;
    int rc = 0;
    opts = fcntl(fd, F_GETFL);

    if(opts < 0)
    {
        rc = E_F_GETFL;
    }
    else
    {
        opts = (opts | O_NONBLOCK);
        if(fcntl(fd, F_SETFL, opts) < 0) 
        {
            rc = E_F_SETFL;
        }
    }

    return(rc);
}

/*
 * *************************************************************
 * Signal handling stuff
 * *************************************************************
*/

typedef void (*sighandler_t)(int);

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
 * new signal handler
 * *************************************************************
*/
static void catch_SIGUSR1(int val)
{
   return;
}

/*
 * *************************************************************
 * display help screen and exit
 * *************************************************************
*/
void help( srv_opts_t *options, char* pgm )
{
   fprintf(stderr, "%s\n", "Usage:");
   fprintf(stderr, "%s\n", "--verbose=<level> --debug=<level> --port=<portno> --foreground --help");
   fprintf(stderr, "%s\n", "where meaning of the several options is:");
   fprintf(stderr, "%s\n",  
      " --verbose	( -v )	verbose level	default = --verbose=0");
   fprintf(stderr, "%s\n",
      " --debug	( -d )	debug level	default = --debug=0");
   fprintf(stderr, "%s\n",
      " --port	( -p )	listener port	default = --port=6633");
   fprintf(stderr, "%s\n",
      " --foreground ( -f )  do not demonize	default = unset, run as daemon");
   fprintf(stderr, "%s\n",
      " --help	( -? ) print this help screen and exit");

	exit(0);

}

/*
 * *************************************************************
 * get command line args
 * *************************************************************
*/
void get_arguments ( int argc, char **argv, srv_opts_t *options )
{

   int next_option;
   short s_opt_val;

   /* valid short options letters */
   const char* const short_options = "fp:d:v:?";

   /* valid long options */
   const struct option long_options[] = {
      { "foreground",		0, NULL, 'f' },
      { "port",			1, NULL, 'p' },
      { "verbose",		1, NULL, 'v' },
      { "debug",		1, NULL, 'd' },
      { "help",			0, NULL, '?' },
      { NULL,			0, NULL,  0  }
   };


   do
   {
      next_option = getopt_long (argc, argv, short_options, 
         long_options, NULL);

      switch (next_option)
      {
         case 'f':
            options->run_as_daemon=0;
            break;
         case 'v':
            options->verbose_level=atoi(optarg);
            break;
         case 'd':
            options->debug_level=atoi(optarg);
            break;
         case 'p':
            options->listen_port=atoi(optarg);
            break;
         case '?':
            help(options, argv[0]);
            break;
         default:
            break;
      }
  } while (next_option != -1);
}

/*
 * *************************************************************
 * Make program to run as a daemon
 * *************************************************************
*/
static int start_daemon( srv_opts_t *options )
{
   int i;
   pid_t pid;
   FILE *fd_pid;
   int error_code = E_NOERR;

   if ((pid = fork ()) != 0)
   {
      if( pid < 0 )
      {
         options->sys_errno = errno;
         error_code = E_FORK;
      }
      else
      {
         // parent may exit now
         exit(0);
      }
   }

   if (setsid() < 0)
   {
      options->sys_errno = errno;
      error_code = E_SETSID;
   }
   else
   {
      handle_signal (SIGHUP, SIG_IGN);

      options->run_mode = RUN_DAEMON;

      chdir ("/");
      umask (0);

      for (i = sysconf (_SC_OPEN_MAX); i > 0; i--)
      {
         close (i);
      }

      openlog ( options->daemon_name, 
                LOG_PID | LOG_CONS| LOG_NDELAY, options->log_facility );

      handle_signal (SIGUSR1, catch_SIGUSR1);

      clock_gettime( CLOCK_REALTIME, &options->start_time );

      if( (fd_pid = fopen( options->pid_file_name, "w+")) != NULL )
      {
         fprintf(fd_pid, "%d", getpid() );
         fclose( fd_pid );
      }
      else
      {
         sprintf( options->logbuffer, "Cannot create PID-File\n");
         do_log(options, LOG_WARN);
      }
   }
   return( error_code );
}

/*
 * *************************************************************
 * Wait for client to connect
 * *************************************************************
*/
int wait_client( srv_opts_t *options )
{
   int error_code = E_NOERR;

   listen(options->socket, options->backlog);

   if( (options->cli_socket = accept(options->socket,
      (struct sockaddr*) &options->remote_addr, &options->sock_len )) >= 0)
   {
//    nonblock(newsockfd);
      error_code = E_NOERR;
   }
   else
   {
      options->sys_errno = errno;
      error_code = E_FAIL;
   }

   return(error_code);
}

/*
 * *************************************************************
 * get request from client
 * *************************************************************
*/
int get_request( srv_opts_t *options )
{
   int error_code = E_NOERR;

   memset(options->p_rcv_buf, 0, sizeof(options->rcv_buf));
   if( (options->rcv_len = read( options->cli_socket, options->p_rcv_buf,
      sizeof(options->rcv_buf)) ) < 0 )
   {
      error_code = E_FAIL;
   }
   else
   {
         // n = write(newsockfd, buffer, n);
   }

   return( error_code );
}

/*
 * *************************************************************
 * send response back to client
 * *************************************************************
*/
int send_response( srv_opts_t *options )
{
   if( options != NULL )
   {
      options->rsp_len = write( options->cli_socket, options->p_rsp_buf,
                                 strlen(options->p_rsp_buf) );
   }
   return( E_NOERR );
}

static known_cmd_t KnownCmd[] = {
{ "Q",	"%c:%02d",			REQ_QUIT },
{ "X",	"%c:%02d",			REQ_EXIT },
{ "T",	"%c:%02d:%d",			REQ_DEBUG },
{ "V",	"%c:%02d:%d",			REQ_VERBOSE },
{ "D",	"%c:%02d:%12s:%d:%c:%c:%c:%c:%c:%s",	REQ_DEVICE },
// e.g.  "D:10:/dev/ttyUSB0:57600:8:N:1:N:0:rw";
{ NULL,	NULL, 0 }
};

/*
 * *************************************************************
 * parse incoming request
 * *************************************************************
*/
int parse_req( char* req )
{
   int reqIdx, i;

   if( req != NULL )
   {
      reqIdx = -1;
      i = 0;

      do
      {
         // check whether request is current item
         // difference between lower and upper case is a blank
         if( req[0] == KnownCmd[i].cmd[0] || 
            (abs(req[0] - KnownCmd[i].cmd[0]) == ' ') )
         {
            reqIdx = i;
         }
         i++;
      } while( reqIdx < 0 && KnownCmd[i].cmd != NULL );
   }
   return reqIdx;
}

// Lookup table for matching entry
// insert new item
// delete an item (search by pid)
// destroy whole table

/*
 * *************************************************************
 * int find_client_by_dev( srv_opts_t *options )
 * *************************************************************
*/
int find_client_by_dev( srv_opts_t *options )
{
   int found;
   int i;

   for(i=0, found=-1; options->childs[i] != (clienttab_t*) NULL && 
       found < 0; i++)
   {
      if( options->childs[i]->device != NULL )
      {
         if( strcasecmp(options->childs[i]->device, 
                    options->serial_dev.device_name) == 0 )
         {
            found = i;
         }
      }
   }

   return(found);
}

/*
 * *************************************************************
 * int find_client_by_pid( pid_t pid, srv_opts_t *options )
 * *************************************************************
*/
int find_client_by_pid( pid_t pid, srv_opts_t *options )
{
   int found;
   int i;

   for(i=0, found=-1; options->childs[i] != (clienttab_t*) NULL && 
       found < 0; i++)
   {
      if( options->childs[i]->device != NULL )
      {
         if( options->childs[i]->pid == pid )
         {
            found = i;
         }
      }
   }

   return(found);
}

/*
 * *************************************************************
 * int add_client( srv_opts_t *options )
 * *************************************************************
*/
int add_client( pid_t pid, srv_opts_t *options )
{
   int error = E_NOERR;

   if( options != NULL )
   {
      if( ( options->childs = realloc( options->childs, 
            (options->child_entry+2 * sizeof(clienttab_t*)) )) != NULL )
      {
         if( ( options->childs[options->child_entry] = 
               realloc( options->childs[options->child_entry], 
                        sizeof(clienttab_t)) ) != NULL )
         {
            options->childs[options->child_entry]->device =
               strdup( options->serial_dev.device_name );
            options->childs[options->child_entry]->mode =
               strdup( options->serial_dev.dev_mode );
            options->childs[options->child_entry]->pid = pid;

            options->child_entry++;
            options->childs[options->child_entry] = NULL;
         }
         else
         {
            error = E_NOMEM;
         }
      }
      else
      {
         error = E_NOMEM;
      }
   }
   else
   {
      error = E_NULLPTR;
   }
   return(error);
}

/*
 * *************************************************************
 * int del_client(int idx, srv_opts_t *options )
 * *************************************************************
*/
void del_client(int idx, srv_opts_t *options )
{
   int i;
   clienttab_t* item;

   if(  options != NULL )
   {
      if(  options->childs != NULL && idx > 0 && idx < options->child_entry )
      {
         item = options->childs[idx-1];
         for( i=idx-1; options->childs[i] != (clienttab_t*) NULL; i++ )
         {
            options->childs[i] = options->childs[i+1];
            options->childs[i+1] = NULL;
         }
         free( item->device );
         free( item->mode );
         free( item );
         options->child_entry--;
      }
   }
}

/*
 * *************************************************************
 * void destroy_client_table( srv_opts_t *options )
 * *************************************************************
*/
void destroy_client_table( srv_opts_t *options )
{
   int i;

   if( options != NULL && options->childs != NULL )
   {
      for(i=0; options->childs[i] != (clienttab_t*) NULL; i++)
      {
         if( options->childs[i]->device != NULL )
         {
            free( options->childs[i]->device );
         }
         if( options->childs[i]->mode != NULL )
         {
            free( options->childs[i]->mode );
         }

         free( options->childs[i] );
      }
   }
}


/*
 * *************************************************************
 * Clean up and terminate with exitcode given as argument
 * *************************************************************
*/
void cleanup_and_exit( srv_opts_t *options )
{
   // cleanup e.g. free allocated pointers
   if( options != NULL )
   {
      destroy_client_table( options );

#ifndef UBUNTU

#ifdef SUBCTL_2SONIC
      i2cClose(options->i2cHandle2Sonic);
#endif // SUBCTL_2SONIC

#ifdef SUBCTL_4SONIC
      i2cClose(options->i2cHandle4Sonic);
#endif // SUBCTL_4SONIC

#ifdef SUBCTL_MOTOR
      i2cClose(options->i2cHandleMotor);
#endif // SUBCTL_MOTOR

#ifdef SUBCTL_GYRO
      i2cClose(options->i2cHandleGyro);
#endif // SUBCTL_GYRO

#ifdef SUBCTL_TFT
      i2cClose(options->i2cHandleTft);
#endif // SUBCTL_TFT

      if( options->pigpioInitialized )
      {
         gpioTerminate();
         options->pigpioInitialized = 0;
      }

#endif // NOT UBUNTU

   }

   // and suspend process
   suspend( options );
}

/*
 * *************************************************************
 * int setup_serial( srv_opts_t *options )
 * *************************************************************
*/
int setup_serial( srv_opts_t *options )
{
   int fail = E_NOERR;
   int retries = 3;

   if( options != NULL )
   {
      if( (fail = rs232Open(&options->serial_dev)) > 0 )
      {
         if( !(fail = rs232GetParam( &options->serial_dev,
            &options->serial_dev.oldsettings )) )
         {
            if( !(fail = rs232GetParam( &options->serial_dev,
               &options->serial_dev.checksettings )) )
            {
               if( !(fail = rs232GetParam( &options->serial_dev,
                  &options->serial_dev.newsettings )) )
               {
                  if( !(fail = rs232SetUpRaw( &options->serial_dev,
                     &options->serial_dev.checksettings )) )
                  {
                     if( !(fail = rs232SetUpRaw( &options->serial_dev,
                        &options->serial_dev.newsettings )) )
                     {
                        if( !(fail = rs232SetUpParam( &options->serial_dev,
                           &options->serial_dev.newsettings )) )
                        {
                           fail = rs232SetUp( &options->serial_dev,
                                 &options->serial_dev.newsettings,
                                 &options->serial_dev.checksettings,
                                 &retries );
                        }
                        else
                        {
                           fail = E_FAIL;
                        }
                     }
                     else
                     {
                        fail = E_FAIL;
                     }
                  }
                  else
                  {
                     fail = E_FAIL;
                  }
               }
               else
               {
                  fail = E_FAIL;
               }
            }
            else
            {
               fail = E_FAIL;
            }
         }
         else
         {
            fail = E_FAIL;
         }
      }
      else
      {
         // failed to open device
         fail = E_FAIL;
      }
   }
   else
   {
      fail = E_NULLPTR;
   }

   return(fail);
}

/*
 * *************************************************************
 * int use_anyway(int tabIdx, srv_opts_t *options )
 * *************************************************************
*/
int use_anyway(int tabIdx, srv_opts_t *options )
{
   int use_ok = 1;


   return(use_ok);
}

/*
 * *************************************************************
 * main of connection server
 * *************************************************************
*/
int main(int argc, char *argv[])
{

   srv_opts_t run_options;
   int status;
   int RequestNum;
   int RequestCmd;
   char cReq;
   short numArgs;
   int argCnt;
   char tmpDevBuf[16];
   char tmpDevMode[16];
   int bd;
   char db;
   char par;
   char sb;
   char hs;
   char as;
   int tabIdx;
   pid_t curr_child;
   int serial_fail;
#ifdef SUBCTL_2SONIC
   int cond2sonic;
#endif // SUBCTL_2SONIC


   initialize( &run_options );
   get_arguments ( argc, argv, &run_options );

   if( run_options.run_as_daemon )
   {
      if( (run_options.error_code = start_daemon( &run_options )) != E_NOERR )
      {
         do_log(&run_options, LOG_CRITERROR);
         cleanup_and_exit( &run_options );
      }
   }

   if( (run_options.error_code = make_socket( &run_options )) != E_NOERR )
   {
      do_log(&run_options, LOG_CRITERROR);
      cleanup_and_exit( &run_options );
   }


// while not stop request
   RequestCmd = 0;
   do
   {
      if( (run_options.error_code = wait_client( &run_options )) != E_NOERR )
      {
         do_log(&run_options, LOG_CRITERROR);
         cleanup_and_exit( &run_options );
      }
      else
      {
         if( (run_options.error_code = get_request( &run_options )) != E_NOERR )
         {
            do_log(&run_options, LOG_CRITERROR);
            cleanup_and_exit( &run_options );
         }
         else
         {
            if( (RequestNum = parse_req( run_options.p_rcv_buf )) >= 0 )
            {
               switch( RequestCmd = KnownCmd[RequestNum].cmdCode )
               {
                  case REQ_QUIT:
                  case REQ_EXIT:
                     sprintf(run_options.logbuffer, "exit");
                     do_log(&run_options, LOG_DBG );
                     // no response necessary ... only quit server
                     break;
                  case REQ_DEBUG:
                     // no response necessary ... only set debuglevel
                     sprintf(run_options.logbuffer, "set debug level");
                     do_log(&run_options, LOG_DBG );
                     argCnt = sscanf( run_options.p_rcv_buf,
                                      KnownCmd[RequestNum].fmt,
                                      &cReq, &numArgs,
                                      &run_options.debug_level );
                     break;
                  case REQ_VERBOSE:
                     // no response necessary ... only set verboselevel
                     sprintf(run_options.logbuffer, "set verbose level");
                     do_log(&run_options, LOG_DBG );
                     argCnt = sscanf( run_options.p_rcv_buf,
                                      KnownCmd[RequestNum].fmt,
                                      &cReq, &numArgs, 
                                      &run_options.verbose_level );
                     break;
                  case REQ_DEVICE:
                     sprintf(run_options.logbuffer, "request device");
                     do_log(&run_options, LOG_DBG );
                     argCnt = sscanf( run_options.p_rcv_buf,
                                      KnownCmd[RequestNum].fmt,
                                      &cReq, &numArgs, tmpDevBuf,
                                      &bd, &db, &par, &sb, &hs, &as,
                                      tmpDevMode );

                     if( argCnt == numArgs )
                     {
                        // request ok ...
                        run_options.serial_dev.device_name = strdup(tmpDevBuf);
                        run_options.serial_dev.baud = bd;
                        run_options.serial_dev.parity = par;
                        run_options.serial_dev.databit = db-'0';
                        run_options.serial_dev.stoppbits = sb-'0';
                        run_options.serial_dev.handshake = hs;
                        run_options.serial_dev.async = as-'0';
                        run_options.serial_dev.dev_mode = strdup(tmpDevMode);

                        sprintf(run_options.p_rsp_buf,
                                "dev=%s, bd=%d, par=%c, db=%d, sb=%d, hs=%c, asy=%d, m=%s\n",
                                run_options.serial_dev.device_name,
                                run_options.serial_dev.baud,
                                run_options.serial_dev.parity,
                                run_options.serial_dev.databit,
                                run_options.serial_dev.stoppbits,
                                run_options.serial_dev.handshake,
                                run_options.serial_dev.async,
                                run_options.serial_dev.dev_mode );
                        send_response(&run_options);
                        // check whether another client has exclusive
                        // access to the requested device
                        // if so, send response to decline request
                        tabIdx = find_client_by_dev( &run_options );
                        if( tabIdx < 0 )
                        {
                           sprintf(run_options.logbuffer, "no entry -> ok");
                           do_log(&run_options, LOG_DBG );
                           // no client has this device in use... 
                           // we can continue
                           if((serial_fail = setup_serial(&run_options)) < 0)
                           {
                              // failed to setup serial device
                              strcpy(run_options.logbuffer,
                                run_options.serial_dev.logbuffer );
                              do_log(&run_options, LOG_DBG );

                              sprintf(run_options.logbuffer,
                                      "setup serial device failed=%s",
                                      strerror(errno) );
                              do_log(&run_options, LOG_DBG );
                              sprintf(run_options.p_rsp_buf,
                                      "%s , errno was %d (%s)\n",
                                      "failed to setup device", errno,
                                      strerror(errno) );
                              send_response(&run_options);
                           }
                           else
                           {

                              // setup serial device ok

                              strcpy(run_options.logbuffer,
                                run_options.serial_dev.logbuffer );
                              do_log(&run_options, LOG_DBG );

                              sprintf(run_options.logbuffer,
                                      "setup serial device ok");
                              do_log(&run_options, LOG_DBG );
                              sprintf(run_options.p_rsp_buf,
                                      "setup serial device ok\n" );
                              send_response(&run_options);
                           }
                        }
                        else
                        {
                           // there is a client using this device
                           // check for modes whether it is possible
                           // to serve the request anyway
                           sprintf(run_options.logbuffer,
                                   "entry present - use anyway?");
                           do_log(&run_options, LOG_DBG );

                           if( use_anyway(tabIdx, &run_options) )
                           {
                              sprintf(run_options.logbuffer,
                                      "use anyway!");
                              do_log(&run_options, LOG_DBG );
                              if((serial_fail = setup_serial(&run_options)) < 0)
                              {
                                 // failed to setup serial device
                                 strcpy(run_options.logbuffer,
                                   run_options.serial_dev.logbuffer );
                                 do_log(&run_options, LOG_DBG );

                                 sprintf(run_options.logbuffer,
                                         "setup serial device failed=%s",
                                         strerror(errno) );
                                 do_log(&run_options, LOG_DBG );
                                 sprintf(run_options.p_rsp_buf,
                                         "%s , errno was %d (%s)\n",
                                         "failed to setup device", errno,
                                          strerror(errno) );
                                 send_response(&run_options);
                              }
                              else
                              {
                                 // setup serial device ok
                                 sprintf(run_options.logbuffer,
                                         "setup serial device ok");
                                 do_log(&run_options, LOG_DBG );
                                 sprintf(run_options.p_rsp_buf,
                                         "setup serial device ok\n" );
                                 send_response(&run_options);
                              }
                           }
                           else
                           {
                              serial_fail = E_FAIL;
                              // device is locked ... do not use
                              sprintf(run_options.logbuffer,
                                      "serial device locked");
                              do_log(&run_options, LOG_DBG );
                              sprintf(run_options.p_rsp_buf,
                                      "serial device locked\n");
                              send_response(&run_options);
                           }
                        }

                        // no errors yet? 
                        // access granted, device accessible and set up
                        // properly

                        if( serial_fail == E_NOERR )
                        {
                           sprintf(run_options.logbuffer,
                                   "no failure yet -> fork");
                           do_log(&run_options, LOG_DBG );
                           sprintf(run_options.p_rsp_buf,
                                   "no failure yet -> fork\n");
                           send_response(&run_options);
                           // fork() child - this is the point of no return

                           if( (curr_child = fork()) > 0 )
                           {
                              // her we are in the parent process
                              // store pid an other information in control table
                              if( add_client( curr_child, &run_options) < 0 )
                              {
                                 sprintf(run_options.p_rsp_buf,
                                         "%s , errno was %d (%s)\n",
                                         "failed to add client", errno,
                                          strerror(errno) );
                                 send_response(&run_options);
                                 // failed to add new entry ...
                                 // kill just forked child.
                                kill(curr_child, SIGKILL);
                              }
                           }
                           else
                           {
                              if( curr_child == 0 )
                              {
                                 // child branch ...
                                 // here we go to serve the client
                                 sprintf(run_options.p_rsp_buf,
                                         "here we go ...\n" );
                                 send_response(&run_options);
                                 return( runService( &run_options ) );
                              }
                              else
                              {
                                 sprintf(run_options.p_rsp_buf,
                                         "%s , errno was %d (%s)\n",
                                         "failed to create service", errno,
                                          strerror(errno) );
                                 send_response(&run_options);
                                 RequestCmd = REQ_QUIT;
                              }
                           }
                        }
                     }
                     else
                     {
                        // invalid request, send response and ignore it
                        sprintf(run_options.p_rsp_buf,
                                "Dont know(1): %s [%d should be %d]\n",
                                run_options.p_rcv_buf,
                                argCnt,numArgs );

                        send_response(&run_options);
                     }
                     RequestCmd = REQ_QUIT;
                     break;
                  default:
                        // invalid request, send response and ignore it
                        sprintf(run_options.p_rsp_buf,
                                "Dont know(2): %s [%d should be %d]\n",
                                run_options.p_rcv_buf,
                                argCnt,numArgs );
                        send_response(&run_options);
                     RequestCmd = REQ_QUIT;
                     break;
                  // KnownCmd[RequestNum].cmd
                  // KnownCmd[RequestNum].fmt
                  // KnownCmd[RequestNum].cmdCode
               }
            }
            else
            {
// ////////////////////////////////////////////////////////////////
// this block handles handles all incoming requests that are
// not related to a rs232 connection or a quit/exit request
// ////////////////////////////////////////////////////////////////
#ifndef UBUNTU

#ifdef SUBCTL_2SONIC
               switch( cond2sonic = check2SonicCondition( &run_options ) )
               {
                  case CONDITION_2SONIC_R:
                     sprintf(run_options.p_rsp_buf,
                             "2sonic says: condition red\n" );
                     send_response(&run_options);
                     break;
                  case CONDITION_2SONIC_Y:
                     sprintf(run_options.p_rsp_buf,
                             "2sonic says: condition yellow\n" );
                     send_response(&run_options);
                     break;
                  case CONDITION_2SONIC_G:
                     sprintf(run_options.p_rsp_buf,
                             "2sonic says: condition green\n" );
                     send_response(&run_options);
                     break;
                  default:
                     break;
               }
#endif // SUBCTL_2SONIC

#else // IS UBUNTU
               // invalid request, send response and ignore it
               sprintf(run_options.p_rsp_buf,
                       "Dont know(3): %s [%d should be %d]\n",
                       run_options.p_rcv_buf,
                       argCnt,numArgs );
               send_response(&run_options);
               RequestCmd = REQ_EXIT;
#endif // NOT UBUNTU
// ////////////////////////////////////////////////////////////////
            }
         }
      }
   } while( RequestCmd != REQ_QUIT && RequestCmd != REQ_EXIT );

// wait for all children to terminate
   while( wait(&status) >= 0 )
      ;

// and die
   cleanup_and_exit( &run_options );

}

