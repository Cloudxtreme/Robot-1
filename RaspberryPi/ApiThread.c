/* *******************************************************************
 *
 * ApiThread.c
 *
 * (C) 2014 Dreamshader (Dirk Schanz)
 *
 *  part of a multi-process server.
 *     stuff that will run as a child process
 *     
 *
 * gcc -c ApiThread.c
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
#include "ApiThread.h"

/*
 * *************************************************************
 * static void *run_tcp_thread()
 * startup the control thread for remote control over tcp
 * *************************************************************
*/
void *run_api_thread(void *runOptions)
{
   srv_opts_t *options = (srv_opts_t*) runOptions;

   if( options != NULL )
   {
      options->ApiThreadStatus = CTRL_THREAD_RUNNING;

      // main loop - never ends
      while( options->api_run )
      {
      }

   }

   return(NULL);
}

#ifdef NEVERDEF
	void *return_value = "NULL";
	int exit_code;
	int loops;
	int old_state;
	int old_type;
	int remotelen;
	int cmd;
	struct soft_pwm_options_t *pwm_ctl;

	pwm_ctl = global_pwm_ctl;
	exit_code = 0;

	pwm_ctl->misc_opt->pwm_tcp_ctl_status = SOFTPWM_THREAD_RUNNING;

	if( (exit_code = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, 
		&old_state)) != 0 )
	{
		return_value = "FAIL";
		exit_code = -1;
	}
	else
	{
		if( (exit_code = pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, 
			&old_type)) != 0 )
		{
			return_value = "FAIL";
			exit_code = -1;
		}
		else
		{
			exit_code = prepare_server_socket(pwm_ctl);
		}
	}

   	for(; exit_code == 0;)
	{


		listen(pwm_ctl->rctl->rctl_listener, 5);
		remotelen = sizeof( pwm_ctl->rctl->rctl_remoteaddr );

		if( (pwm_ctl->rctl->rctl_cmd_interface = 
			accept(pwm_ctl->rctl->rctl_listener,
			(struct sockaddr *) &pwm_ctl->rctl->rctl_remoteaddr,
			&remotelen)) < 0 )
		{
			return_value = "FAIL";
			exit_code = -1;
			break;
		}


		bzero(pwm_ctl->rctl->p_rctl_recv_buf, 
			sizeof(pwm_ctl->rctl->rctl_recv_buf) );

		rctl_prompt( pwm_ctl );

		while( read( pwm_ctl->rctl->rctl_cmd_interface, 
				pwm_ctl->rctl->p_rctl_recv_buf, 
				sizeof(pwm_ctl->rctl->rctl_recv_buf)-1) >= 0 )
		{
			cmd = do_tcp_command( pwm_ctl );

			rctl_prompt( pwm_ctl );

			if( cmd == SOFTPWM_TCP_CMD_CLOSE )
			{
				exit_code = 0;
				break;
			}

			if( cmd == SOFTPWM_TCP_CMD_EXIT )
			{
				exit_code = 2;
				break;
			}

		}

		close( pwm_ctl->rctl->rctl_cmd_interface );
	}

	return_value = exit_tcp_thread( exit_code );
static void *exit_tcp_thread( int exit_code )
{
	struct soft_pwm_options_t *pwm_ctl;

	pwm_ctl = global_pwm_ctl;
	pwm_ctl->misc_opt->pwm_tcp_ctl_status = SOFTPWM_THREAD_FINISHED;
//	pwm_ctl->misc_opt->pwm_tcp_ctl_status = SOFTPWM_THREAD_FAILED;


	return( (void*) "NULL" );
}


	return( return_value );
}

#endif // NEVERDEF

