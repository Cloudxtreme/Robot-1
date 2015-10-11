/* *******************************************************************
 *
 * MasterControl.c
 *
 * (C) 2014 Dreamshader (Dirk Schanz)
 *
 * main control program for robot
 *     runs as a daemon in the background and manage all arduino
 *     pro mini subcontroller connected by the IIC bus.
 *     A simple tcp/ip server is listening for incoming connections,
 *     too. This server is used in two ways:
 *     A human supervisor may connect to it using telnet. All logging
 *     and informational output of the several subcontrollers is done
 *     using this connection. So it is possible to verify the proper
 *     working of the controlling process. In addition it is possible
 *     to override control of the main controller process by using this
 *     connection and send commands to a specific subcontroller.
 *     The tcp/ip connection is also used by the graphical interface.
 *
 * gcc -o MasterControl MasterControl.c -lpigpio -lpthread -lrt
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
#include <pthread.h>
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

#include "MasterControl.h"
#include "ApiThread.h"
#include "TelnetThread.h"
#include "MainCtlThread.h"

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
         fprintf(stderr, "%s  %s[%d]: %s\n", buff, options->daemon_name, 
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

      options->mainctl_run = 1;
      options->api_run = 1;
      options->telnet_run = 1;

      options->tinfo = NULL;
      memset(&options->tattr, '\0', sizeof(options->tattr) );
      options->stack_size = DEFAULT_STACKSIZE;

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
            options->run_mode = RUN_FOREGROUND;
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


static master_cmd_t MasterCmd[] = {
{ "X",		CTLREQ_EXIT },
{ "x",		CTLREQ_EXIT },
{ "%",		CTLREQ_API },
{ NULL,		0 }
};

/*
 * *************************************************************
 * parse incoming request
 * *************************************************************
*/
int parse_req( char* req )
{
   int reqIdx, i, CommandCode;

   if( req != NULL )
   {
      CommandCode = CTLREQ_UNKNOWN;
      reqIdx = -1;
      i = 0;
fprintf(stderr, "\ni=%d, req[0]=%c, CommandCode=%d reqIdx=%d\n", i, req[0], CommandCode, reqIdx);

      do
      {
         // check whether request is current item
         // difference between lower and upper case is a blank
         if(req[0] == MasterCmd[i].cmd[0]) 
         {
            reqIdx = i;
            CommandCode = MasterCmd[i].cmdCode;
fprintf(stderr, "found: %s = %s\n", req, MasterCmd[i].cmd);
         }
         i++;
      } while( reqIdx < 0 && MasterCmd[i].cmd != NULL );

   }
//fprintf(stderr, "i=%d, req[0]=%c, MasterCmd[reqIdx].cmd[0]=%c \n", i, req[0], MasterCmd[reqIdx].cmd[0] );
fprintf(stderr, "i=%d, req[0]=%c, CommandCode=%d reqIdx=%d\n", i, req[0], CommandCode, reqIdx);

   return CommandCode;
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
   }

   // and suspend process
   suspend( options );
}


/*
 * *************************************************************
 * prepare to create threads
 * int prepareThreads( srv_opts_t *options )
 * *************************************************************
*/
int prepareThreads( srv_opts_t *options )
{
   int errcode = E_NOERR;

   if( options != NULL )
   {
      /* Initialize thread creation attributes */
      if( pthread_attr_init(&options->tattr) != 0 )
      {
         errcode = E_FAIL;
         options->error_code = E_FAIL;
         options->sys_errno = errno;
         sprintf( options->logbuffer, "attr_init failed, errno was %d\n", errno );
      }
      else
      {
         if( pthread_attr_setstacksize(&options->tattr, options->stack_size) != 0 )
         {
            errcode = E_FAIL;
            options->error_code = E_FAIL;
            options->sys_errno = errno;
            sprintf( options->logbuffer, "attr_setstacksize failed, errno was %d\n", errno );
         }
         else
         {
            /* Allocate memory for pthread_create() arguments */
            if( (options->tinfo = calloc(MASTERCTL_NUM_THREADS, sizeof(struct thread_info))) == NULL )
            {
               errcode = E_FAIL;
               options->error_code = E_FAIL;
               options->sys_errno = errno;
               sprintf( options->logbuffer, "calloc tinfo failed, errno was %d\n", errno );
            }
         }
      }
   }
   else
   {
      errcode = E_NULLPTR;
   }

   return(errcode);
}

/*
 * *************************************************************
 * create a thread
 * int ctl_create_thread( srv_opts_t *options, int threadId )
 * *************************************************************
*/
int ctl_create_thread( srv_opts_t *options, int threadId )
{

   int errcode;
  
   if( options != NULL )
   {
      options->tinfo[threadId].thread_num = threadId + 1;
      options->tinfo[threadId].options = (void*) options;

      switch( threadId )
      {

         case MAINCTL_THREAD_NUM:
            options->MainThreadStatus = CTRL_THREAD_INIT;
            errcode = pthread_create( &options->tinfo[threadId].thread_id,
                                      &options->tattr,
                                      &run_main_thread,
                                      &options->tinfo[threadId]);
            break;
         case API_THREAD_NUM:
            options->ApiThreadStatus = CTRL_THREAD_INIT;
            errcode = pthread_create( &options->tinfo[threadId].thread_id,
                                      &options->tattr,
                                      &run_api_thread,
                                      &options->tinfo[threadId]);
            break;
         case TELNET_THREAD_NUM:
            options->TelnetThreadStatus = CTRL_THREAD_INIT;
            errcode = pthread_create( &options->tinfo[threadId].thread_id,
                                      &options->tattr,
                                      &run_telnet_thread,
                                      &options->tinfo[threadId]);
            break;
         default:
            errcode = E_THREADINVAL;
            break;
      }

      if( errcode == E_NOERR )
      {
         if((errcode=pthread_detach(options->tinfo[threadId].thread_id)) != 0 )
         {
            errcode = E_FAIL;
            options->error_code = E_FAIL;
            options->sys_errno = errno;
            sprintf( options->logbuffer, "thread detach failed, errno was %d\n", errno );
         }
      }
      else
      {
         errcode = E_FAIL;
         options->error_code = E_FAIL;
         options->sys_errno = errno;
         sprintf( options->logbuffer, "create thread failed, errno was %d\n", errno );
      }

   }
   else
   {
      errcode = E_NULLPTR;
      options->error_code = E_FAIL;
      sprintf( options->logbuffer, "NULLP argument\n" );
   }

   return( errcode );
}



/*
 * *************************************************************
 * main of connection server
 * *************************************************************
*/
int main(int argc, char *argv[])
{

   srv_opts_t run_options;
   int RequestCmd;
   void *res;


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


   if( prepareThreads( &run_options ) != E_NOERR )
   {
         do_log(&run_options, LOG_CRITERROR);
         cleanup_and_exit( &run_options );
   }
   else
   {
      if( (run_options.error_code = ctl_create_thread( &run_options, MAINCTL_THREAD_NUM)) != E_NOERR )
      {
         do_log(&run_options, LOG_CRITERROR);
         cleanup_and_exit( &run_options );
      }
   }

// while no exit request
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
            switch( RequestCmd =  parse_req( run_options.p_rcv_buf ) )
            {
               case CTLREQ_EXIT:
                  sprintf(run_options.p_rsp_buf, "Assume exit: %s\n", run_options.p_rcv_buf );
                  send_response(&run_options);
                  close(run_options.cli_socket);
                  break;
               case CTLREQ_API:
                  sprintf(run_options.p_rsp_buf, "Assume API: %s\n", run_options.p_rcv_buf );
                  send_response(&run_options);
                  close(run_options.cli_socket);
                  break;
               default:
                  // assume telnet request
                  sprintf(run_options.p_rsp_buf, "Assume telnet: %s\n", run_options.p_rcv_buf );
                  send_response(&run_options);
                  close(run_options.cli_socket);
                  break;
            }
         }
      }

   } while( RequestCmd != CTLREQ_EXIT );

   if( pthread_attr_destroy(&run_options.tattr) != 0 )
   {
      run_options.error_code = E_FAIL;
      run_options.sys_errno = errno;
      sprintf( run_options.logbuffer, "destroy attr failed, errno was %d\n", errno );
      do_log(&run_options, LOG_WARN);
   }

   run_options.mainctl_run = 0;
   run_options.api_run = 0;
   run_options.telnet_run = 0;

   if( run_options.tinfo[MAINCTL_THREAD_NUM].thread_id >= 0 )
   {
      pthread_join(run_options.tinfo[MAINCTL_THREAD_NUM].thread_id, &res);
   }

   if( run_options.tinfo[API_THREAD_NUM].thread_id >= 0 )
   {
      pthread_join(run_options.tinfo[API_THREAD_NUM].thread_id, &res);
   }

   if( run_options.tinfo[TELNET_THREAD_NUM].thread_id >= 0 )
   {
      pthread_join(run_options.tinfo[TELNET_THREAD_NUM].thread_id, &res);
   }

   pthread_cancel(run_options.tinfo[MAINCTL_THREAD_NUM].thread_id);
   pthread_cancel(run_options.tinfo[API_THREAD_NUM].thread_id);
   pthread_cancel(run_options.tinfo[TELNET_THREAD_NUM].thread_id);

   // and die
   cleanup_and_exit( &run_options );
}




