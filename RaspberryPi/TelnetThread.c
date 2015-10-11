/* *******************************************************************
 *
 * TelnetThread.c
 *
 * (C) 2014 Dreamshader (Dirk Schanz)
 *
 *  part of a multi-process server.
 *     stuff that will run as a child process
 *     
 *
 * gcc -c TelnetThread.c
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
#include "TelnetThread.h"

/*
 * *************************************************************
 * stuff for the telnet command thread
 * void *run_telnet_thread(void *runOptions)
 * *************************************************************
*/
void *run_telnet_thread(void *runOptions)
{
   srv_opts_t *options = (srv_opts_t*) runOptions;

   if( options != NULL )
   {
      options->TelnetThreadStatus = CTRL_THREAD_RUNNING;

      // main loop - never ends
      while( options->telnet_run )
      {
      }
   }

   return(NULL);
}

