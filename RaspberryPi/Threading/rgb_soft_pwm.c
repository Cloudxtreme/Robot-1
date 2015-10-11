/*
 ***********************************************************************
 *
 *  rgb_soft_pwm.c - simple test program to control an RGB LED with
 *  the GPIOs of Raspberry Pi using the pigpio-API
 *
 *  Copyright (C) 2013 Dreamshader (Dirk Schanz)
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
 *
 *
 ***********************************************************************
 */

#include <pthread.h>
#include <getopt.h>
#include <stdio.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <linux/limits.h>
#include <netinet/in.h>

#include "pigpio.h"

#include "rgb_soft_pwm.h"


struct soft_pwm_options_t *global_pwm_ctl;
struct cmd_list_t cmd[] = {
		{ "r=", "rchannel=",    "value. Set follwos", 
			SOFTPWM_TCP_CMD_SET_RINTENSITY},
		{ "g=", "gchannel=",    "value. Set follwos", 
			SOFTPWM_TCP_CMD_SET_GINTENSITY},
		{ "b=", "bchannel=",    "value. Set follwos", 
			SOFTPWM_TCP_CMD_SET_BINTENSITY},
		{ "g+", "incgchannel",  "follwos", 
			SOFTPWM_TCP_CMD_INCR_GINTENSITY},
		{ "b+", "incbchannel",  "follwos", 
			SOFTPWM_TCP_CMD_INCR_BINTENSITY},
		{ "r-", "decrchannel",  "follwos", 
			SOFTPWM_TCP_CMD_DECR_RINTENSITY},
		{ "g-", "decgchannel",  "follwos", 
			SOFTPWM_TCP_CMD_DECR_GINTENSITY},
		{ "b-", "decbchannel",  "follwos", 
			SOFTPWM_TCP_CMD_DECR_BINTENSITY},

		{ "R-", "decrrfreq",  "follwos", 
			SOFTPWM_TCP_CMD_DECR_RFREQ},
		{ "G-", "decgfreq",  "follwos", 
			SOFTPWM_TCP_CMD_DECR_GFREQ},
		{ "B-", "decbfreq",  "follwos", 
			SOFTPWM_TCP_CMD_DECR_BFREQ},
		{ "R+", "incrfreq",  "follwos", 
			SOFTPWM_TCP_CMD_INCR_RFREQ},
		{ "G+", "incgfreq",  "follwos", 
			SOFTPWM_TCP_CMD_INCR_GFREQ},
		{ "B+", "incbfreq",  "follwos", 
			SOFTPWM_TCP_CMD_INCR_BFREQ},
		{ "R=", "setrfreq",  "follwos", 
			SOFTPWM_TCP_CMD_SET_RFREQ},
		{ "G=", "setgfreq",  "follwos", 
			SOFTPWM_TCP_CMD_SET_GFREQ},
		{ "B=", "setgbreq",  "follwos", 
			SOFTPWM_TCP_CMD_SET_BFREQ},
		{ "i=", "setincr=",      "value. Set follwos", 
			SOFTPWM_TCP_CMD_SET_INCR},
		{ "d=", "setdecr=",      "value. Set follwos", 
			SOFTPWM_TCP_CMD_SET_DECR},
		{ "R", "getrfreq",  "follwos", 
			SOFTPWM_TCP_CMD_GET_RFREQ},
		{ "G", "getgfreq",  "follwos", 
			SOFTPWM_TCP_CMD_GET_GFREQ},
		{ "B", "getbfreq",  "follwos", 
			SOFTPWM_TCP_CMD_GET_BFREQ},
		{ "q", "quit", "close telnet connection", 
			SOFTPWM_TCP_CMD_CLOSE },
		{ "x", "exit", "close telnet connection, terminate daemon", 
			SOFTPWM_TCP_CMD_EXIT},
		{ "?",  "help",         "show this help screen", 
			SOFTPWM_TCP_CMD_HELP},
		{ "r",  "rchannel",     "follwos", 
			SOFTPWM_TCP_CMD_GET_RINTENSITY},
		{ "g",  "gchannel",     "follwos", 
			SOFTPWM_TCP_CMD_GET_GINTENSITY},
		{ "b",  "bchannel",     "follwos", 
			SOFTPWM_TCP_CMD_GET_BINTENSITY},
		{ "i",  "getincr",      "follwos", 
			SOFTPWM_TCP_CMD_GET_INCR},
		{ "d",  "getdecr",      "follwos", 
			SOFTPWM_TCP_CMD_GET_DECR},
		{ "a",  "all",          "follwos", 
			SOFTPWM_TCP_CMD_GET_ALL},
		{ NULL, NULL, NULL, 0}
		};


/* **** rgb_soft_pwm.c *************************************************************
 * 
 * brief description:
 * generate a software pulse wave on three GIOs given as an argument
 * with a frequency given as an argument, too.
 * The idea is to drive an RGB LED using it as a status display.
 * 
 * *********************************************************************************
 * needs: pigpio-Library (http://abyz.co.uk/rpi/pigpio/pigpio.tar)
 * Compile:
 *
 * gcc -o rgb_soft_pwm rgb_soft_pwm.c -I /usr/local/include -L /usr/local/lib
 * 		-l pigpio -l rt -l pthread
 * 
 * *********************************************************************************
 * Options:
 *
 * ------------------------------- Phys. pins to use -------------------------------
 *
 * pin-number means the physical pin-number of the pin at the GPIO-Header for the
 * red, green and resp. blue channel.
 *
 *   --pin_r GPIO-# (same as --pin_r=GPIO-# resp. -r pin-#)
 *   --pin_g GPIO-# (same as --pin_g=GPIO-# resp. -g pin-#)
 *   --pin_b GPIO-# (same as --pin_b=GPIO-# resp. -b pin-#)
 *  ( pin-# must be 3, 5, 7, 11, 13, 15, 19, 21, 23, 8, 10, 12, 16, 18, 22, 24 or 26)
 *
 * Note, that only "pure" I/O pins (pins without any other special usable
 * funtionality like Rx/Tx, clocks ...) can safely be used as PWM output
 *
 *   Default for the physical pins are #16 (GPIO23), #18 (GPIO24) and #22 (GPIO25) for 
 *                                        red,         green and         blue.
 *
 * ----------------------------- Intensity of the PWM ------------------------------
 *
 * --intensityR (same as -r)
 *     Intensity of red channel 0 - 255
 *   
 * --intensityG (same as -g)
 *     Intensity of green channel 0 - 255
 *   
 * --intensityB (same as -b)
 *     Intensity of blue channel 0 - 255
 *   
 *
 * Defaults are -r 128 -g 128 -b 128
 *
 * ----------------------------- Set config file ------------------------------
 *
 * --config (same as -c)
 * 
 * Path to the config-file
 *   
 * Default is ~/.softpwm.cfg
 *
 * ---------------------------- Info level for logging -----------------------------
 *
 * --verbose level (same as --verbose=level resp. -v level)
 *
 *   Default verbose level is --verbose=4
 *
 * ------------------------- port for rctl to listen -------------------------------
 *
 * --port port (same as --port=port resp. -p port)
 *
 *   Default is --port=3434
 *
 * ----------------------------- name of lockfile ----------------------------------
 *
 * --lock path (same as --lock=path resp. -L path)
 *
 *   Default verbose level is --verbose=4
 *
 * *********************************************************************************
*/


/* ---------------------------------------------------------------------------------
 | void set_pwm_default_options( struct soft_pwm_options_t *pwm_ctl )
 |
 | reset all parameters to their default values
 -----------------------------------------------------------------------------------
*/

void set_pwm_default_options( struct soft_pwm_options_t *pwm_ctl )
{


	pwm_ctl->r_channel->intensity =  DEFAULT_INTENSITY_R;
	pwm_ctl->r_channel->frequency =  DEFAULT_FREQUENCY_R;
	pwm_ctl->r_channel->f_multi   =  SOFTPWM_KHZ_MULTI;
	pwm_ctl->r_channel->pin = atoi( DEFAULT_GPIONAME_R );
	pwm_ctl->g_channel->intensity =  DEFAULT_INTENSITY_G;
	pwm_ctl->g_channel->frequency =  DEFAULT_FREQUENCY_G;
	pwm_ctl->g_channel->f_multi   =  SOFTPWM_KHZ_MULTI;
	pwm_ctl->g_channel->pin = atoi( DEFAULT_GPIONAME_G );
	pwm_ctl->b_channel->intensity =  DEFAULT_INTENSITY_B;
	pwm_ctl->b_channel->frequency =  DEFAULT_FREQUENCY_B;
	pwm_ctl->b_channel->f_multi   =  SOFTPWM_KHZ_MULTI;
	pwm_ctl->b_channel->pin = atoi( DEFAULT_GPIONAME_B );

	pwm_ctl->rctl->rctl_serveraddr.sin_port = htons(DEFAULT_PWM_LISTENPORT);
	pwm_ctl->rctl->rctl_serveraddr.sin_family = AF_INET;
	pwm_ctl->rctl->rctl_serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

	pwm_ctl->rctl->p_rctl_recv_buf = &pwm_ctl->rctl->rctl_recv_buf[0];
	pwm_ctl->rctl->p_rctl_resp_buf=  &pwm_ctl->rctl->rctl_resp_buf[0];
	pwm_ctl->rctl->p_rctl_logbuffer= &pwm_ctl->rctl->rctl_logbuffer[0];

	pwm_ctl->misc_opt->p_log = &pwm_ctl->misc_opt->logbuffer[0];
	pwm_ctl->misc_opt->p_cfg = &pwm_ctl->misc_opt->config_file_name[0];

	pwm_ctl->misc_opt->inc_val = DEFAULT_PWM_INCVAL;
	pwm_ctl->misc_opt->dec_val = DEFAULT_PWM_DECVAL;
	pwm_ctl->misc_opt->board_revision = DEFAULT_PWM_BOARDREV;
	pwm_ctl->misc_opt->log_facility = DEFAULT_PWM_LOGFACILITY;
	pwm_ctl->misc_opt->log_options = DEFAULT_PWM_LOGOPTIONS;
	pwm_ctl->misc_opt->daemon_name = strdup(DEFAULT_PWM_DAEMONNAME);
	pwm_ctl->misc_opt->verbose_level = DEFAULT_PWM_VLEVEL;
	pwm_ctl->misc_opt->stack_size = DEFAULT_STACKSIZE;
	pwm_ctl->misc_opt->pwm_r_ctl_status = 0;
	pwm_ctl->misc_opt->pwm_g_ctl_status = 0;
	pwm_ctl->misc_opt->pwm_b_ctl_status = 0;
	pwm_ctl->misc_opt->pwm_tcp_ctl_status = 0;
	pwm_ctl->misc_opt->pid_file_name = strdup(DEFAULT_PWM_PIDFILENAME);
	strcpy(pwm_ctl->misc_opt->config_file_name, DEFAULT_PWM_CFGFILENAME);

	clock_gettime( CLOCK_REALTIME, &pwm_ctl->misc_opt->call_time );
	clock_gettime( CLOCK_REALTIME, &pwm_ctl->misc_opt->start_time );
	clock_gettime( CLOCK_REALTIME, &pwm_ctl->misc_opt->end_time );

}

/* ---------------------------------------------------------------------------------
 | void clear_pwm_options( struct soft_pwm_options_t *pwm_ctl )
 |
 | reset all parameters to zero
 -----------------------------------------------------------------------------------
*/

void clear_pwm_options( struct soft_pwm_options_t *pwm_ctl )
{

	if( pwm_ctl != (struct soft_pwm_options_t *) NULL )
	{
		if( pwm_ctl->r_channel != NULL )
		{
			pwm_ctl->r_channel->intensity = 0;
			pwm_ctl->r_channel->frequency = 0;
			pwm_ctl->r_channel->f_multi   = 0;
			pwm_ctl->r_channel->pin = 0;
		}

		if( pwm_ctl->g_channel != NULL )
		{
			pwm_ctl->g_channel->intensity = 0;
			pwm_ctl->g_channel->frequency = 0;
			pwm_ctl->g_channel->f_multi   = 0;
			pwm_ctl->g_channel->pin = 0;
		}

		if( pwm_ctl->b_channel != NULL )
		{
			pwm_ctl->b_channel->intensity = 0;
			pwm_ctl->b_channel->frequency = 0;
			pwm_ctl->b_channel->f_multi   = 0;
			pwm_ctl->b_channel->pin = 0;
		}


		if( pwm_ctl->rctl != NULL )
		{
			pwm_ctl->rctl->rctl_listener = -1;
			pwm_ctl->rctl->rctl_cmd_interface = -1;

			memset(&pwm_ctl->rctl->rctl_serveraddr, 0x00, sizeof(struct sockaddr_in));
			memset(&pwm_ctl->rctl->rctl_remoteaddr, 0x00, sizeof(struct sockaddr_in));

			memset(&pwm_ctl->rctl->rctl_timeout, '\0',
				sizeof(pwm_ctl->rctl->rctl_timeout) );
			pwm_ctl->rctl->rctl_errno = 0;

			pwm_ctl->rctl->p_rctl_recv_buf = NULL;
			memset( pwm_ctl->rctl->rctl_recv_buf, '\0', 
				sizeof( pwm_ctl->rctl->rctl_recv_buf ) );
			pwm_ctl->rctl->p_rctl_resp_buf= NULL;
			memset( pwm_ctl->rctl->rctl_resp_buf, '\0', 
				sizeof( pwm_ctl->rctl->rctl_resp_buf ) );
			pwm_ctl->rctl->p_rctl_logbuffer= NULL;
			memset( pwm_ctl->rctl->rctl_logbuffer, '\0', 
				sizeof( pwm_ctl->rctl->rctl_logbuffer ) );
		}


		if( pwm_ctl->misc_opt != NULL )
		{


			pwm_ctl->misc_opt->tinfo = NULL;
			memset(&pwm_ctl->misc_opt->attr, '\0', 
				sizeof(pwm_ctl->misc_opt->attr) );
			pwm_ctl->misc_opt->stack_size = 0;

			pwm_ctl->misc_opt->board_revision =  0;
			pwm_ctl->misc_opt->log_facility =  0;
			pwm_ctl->misc_opt->log_options =  0;
			pwm_ctl->misc_opt->daemon_name = NULL;
			pwm_ctl->misc_opt->daemon_pid = 0;
			memset( &pwm_ctl->misc_opt->call_time, '\0', 
				sizeof( pwm_ctl->misc_opt->call_time ) );
			memset( &pwm_ctl->misc_opt->start_time, '\0', 
				sizeof( pwm_ctl->misc_opt->start_time ) );
			memset( &pwm_ctl->misc_opt->end_time, '\0', 
				sizeof( pwm_ctl->misc_opt->end_time ) );
			pwm_ctl->misc_opt->exit_code = 0;
			pwm_ctl->misc_opt->last_error = 0;
			pwm_ctl->misc_opt->verbose_level = 0;
			pwm_ctl->misc_opt->pid_file_name = NULL;
			memset( pwm_ctl->misc_opt->logbuffer, '\0', 
				sizeof( pwm_ctl->misc_opt->logbuffer ) );
			memset( &pwm_ctl->misc_opt->config_file_name[0], '\0',
				sizeof( pwm_ctl->misc_opt->config_file_name ) );
		}
	}
}

/* ---------------------------------------------------------------------------------
 | void get_arguments ( int argc, char **argv, struct soft_pwm_options_t *pwm_ctl )
 |
 | scan commandline for arguments an set the corresponding values for the program
 -----------------------------------------------------------------------------------
*/

void get_arguments ( int argc, char **argv, struct soft_pwm_options_t *pwm_ctl )
{

	int next_option;
	short s_opt_val;
	int pin_val;
	int gpio_val;
	int bcm_val;

	/* valid short options letters */
	const char* const short_options = "r:g:b:R:G:B:F:v:p:P:c:L:?";

	/* valid long options */
	const struct option long_options[] = {
 		{ "pin_r",		1, NULL, 'r' },
 		{ "pin_g",		1, NULL, 'g' },
 		{ "pin_b",		1, NULL, 'b' },
 		{ "frequencyR",		1, NULL, 'R' },
 		{ "frequencyG",		1, NULL, 'G' },
 		{ "frequencyB",		1, NULL, 'B' },
 		{ "verbose",		1, NULL, 'v' },
 		{ "port",		1, NULL, 'p' },
 		{ "config",		1, NULL, 'c' },
 		{ "lock",		1, NULL, 'L' },
 		{ "help",		0, NULL, '?' },
		{ NULL,			0, NULL,  0  }
	};


	do
	{
		next_option = getopt_long (argc, argv, short_options,
			long_options, NULL);

		switch (next_option) {
			case 'L':
				if( pwm_ctl->misc_opt->pid_file_name != NULL )
				{
					free(pwm_ctl->misc_opt->pid_file_name);
				}
				pwm_ctl->misc_opt->pid_file_name = strdup(optarg);
				break;
			case 'r':
			case 'g':
			case 'b':
				pin_val = atoi(optarg);
				if( (bcm_val = pin2bcm( pin_val )) )
				{
					if( next_option == 'r' )
					{
						pwm_ctl->r_channel->pin = bcm_val;
					}
					else
					{
						if( next_option == 'g' )
						{
							pwm_ctl->g_channel->pin = bcm_val;
						}
						else
						{
							pwm_ctl->b_channel->pin = bcm_val;
						}
					}
				}
				break;
			case 'R':
				pwm_ctl->r_channel->intensity = atoi(optarg);
				break;
			case 'G':
				pwm_ctl->g_channel->intensity = atoi(optarg);
				break;
			case 'B':
				pwm_ctl->b_channel->intensity = atoi(optarg);
				break;
			case 'v':
				pwm_ctl->misc_opt->verbose_level = atoi(optarg);
				break;
			case 'p':
				gpio_val = atoi(optarg);
				break;
			case 'c':
				strcpy(pwm_ctl->misc_opt->config_file_name, optarg);
				break;
			case '?':
				// display_usage();
				break;
			case -1:
				break;
			default:
				fprintf(stderr, "Invalid option %c! \n", next_option);
		}
		// fprintf( stderr, "Option %c -> value %d \n", next_option, gpio_val);
	} while (next_option != -1);
}

/* ---------------------------------------------------------------------------------
 | struct soft_pwm_options_t *soft_pwm_release( struct soft_pwm_options_t *pwm_ctl )
 |
 | free all allocated resources, close sockets and cleanup
 -----------------------------------------------------------------------------------
*/

struct soft_pwm_options_t *soft_pwm_release( struct soft_pwm_options_t *pwm_ctl )
{
	struct soft_pwm_options_t *retval = NULL;

	if( pwm_ctl != NULL )
	{
		if( pwm_ctl->r_channel != NULL )
		{
			free( (void*) pwm_ctl->r_channel );
		}

		if( pwm_ctl->g_channel != NULL )
		{
			free( (void*) pwm_ctl->g_channel );
		}

		if( pwm_ctl->b_channel != NULL )
		{
			free( (void*) pwm_ctl->b_channel );
		}

		if( pwm_ctl->rctl != NULL )
		{
			free( (void*) pwm_ctl->rctl );
		}

		if( pwm_ctl->misc_opt != NULL )
		{
			free( (void*) pwm_ctl->misc_opt );
		}

		free( (void*) pwm_ctl );
		retval = NULL;
	}

	return( retval );
}

/* ---------------------------------------------------------------------------------
 | void soft_pwm_showargs( struct soft_pwm_options_t *pwm_ctl )
 |
 | dump all parameters to screen
 -----------------------------------------------------------------------------------
*/

void soft_pwm_showargs( struct soft_pwm_options_t *pwm_ctl )
{
	unsigned char outbuf[1024];

	memset(outbuf, '\0', sizeof(outbuf));

	sprintf(outbuf, "pwm_ctl->r_channel->intensity ......: %d",
		pwm_ctl->r_channel->intensity );
	soft_pwm_log(outbuf, LOG_DUMP);
	sprintf(outbuf, "pwm_ctl->r_channel->frequency ......: %d",
		pwm_ctl->r_channel->frequency );
	soft_pwm_log(outbuf, LOG_DUMP);
	sprintf(outbuf, "pwm_ctl->r_channel->pin ............: %d",
		pwm_ctl->r_channel->pin );
	soft_pwm_log(outbuf, LOG_DUMP);

	sprintf(outbuf, "pwm_ctl->g_channel->intensity ......: %d",
		pwm_ctl->g_channel->intensity );
	soft_pwm_log(outbuf, LOG_DUMP);
	sprintf(outbuf, "pwm_ctl->g_channel->frequency ......: %d",
		pwm_ctl->g_channel->frequency );
	soft_pwm_log(outbuf, LOG_DUMP);
	sprintf(outbuf, "pwm_ctl->g_channel->pin ............: %d",
		pwm_ctl->g_channel->pin );
	soft_pwm_log(outbuf, LOG_DUMP);

	sprintf(outbuf, "pwm_ctl->b_channel->intensity ......: %d",
		pwm_ctl->b_channel->intensity );
	soft_pwm_log(outbuf, LOG_DUMP);
	sprintf(outbuf, "pwm_ctl->b_channel->frequency ......: %d",
		pwm_ctl->b_channel->frequency );
	soft_pwm_log(outbuf, LOG_DUMP);
	sprintf(outbuf, "pwm_ctl->b_channel->pin ............: %d",
		pwm_ctl->b_channel->pin );
	soft_pwm_log(outbuf, LOG_DUMP);

	sprintf(outbuf, "pwm_ctl->rctl->rctl_listener .......: %d",
		pwm_ctl->rctl->rctl_listener );
	soft_pwm_log(outbuf, LOG_DUMP);
	sprintf(outbuf, "pwm_ctl->rctl->rctl_cmd_interface ..: %d",
		pwm_ctl->rctl->rctl_cmd_interface );
	soft_pwm_log(outbuf, LOG_DUMP);
	sprintf(outbuf, "pwm_ctl->rctl->rctl_serveraddr .....: %x",
		ntohl(pwm_ctl->rctl->rctl_serveraddr.sin_addr.s_addr) );
	soft_pwm_log(outbuf, LOG_DUMP);
	sprintf(outbuf, "pwm_ctl->rctl->rctl_serveraddr Port : %x",
		ntohs(pwm_ctl->rctl->rctl_serveraddr.sin_port) );
	soft_pwm_log(outbuf, LOG_DUMP);
	sprintf(outbuf, "pwm_ctl->rctl->rctl_remoteaddr .....: %x",
		ntohl(pwm_ctl->rctl->rctl_remoteaddr.sin_addr.s_addr) );
	soft_pwm_log(outbuf, LOG_DUMP);
	sprintf(outbuf, "pwm_ctl->rctl->rctl_remoteaddr Port : %x",
		ntohs(pwm_ctl->rctl->rctl_remoteaddr.sin_port) );
	soft_pwm_log(outbuf, LOG_DUMP);
	sprintf(outbuf, "pwm_ctl->rctl->rctl_timeout ........: %ld:%ld",
		pwm_ctl->rctl->rctl_timeout.tv_sec,
		pwm_ctl->rctl->rctl_timeout.tv_usec );
	soft_pwm_log(outbuf, LOG_DUMP);
	sprintf(outbuf, "pwm_ctl->rctl->p_rctl_recv_buf .....: %x",
		(unsigned int) pwm_ctl->rctl->p_rctl_recv_buf );
	soft_pwm_log(outbuf, LOG_DUMP);
	sprintf(outbuf, "pwm_ctl->rctl->rctl_recv_buf .......: %s",
		pwm_ctl->rctl->rctl_recv_buf );
	soft_pwm_log(outbuf, LOG_DUMP);
	sprintf(outbuf, "pwm_ctl->rctl->p_rctl_resp_buf .....: %x",
		(unsigned int) pwm_ctl->rctl->p_rctl_resp_buf );
	soft_pwm_log(outbuf, LOG_DUMP);
	sprintf(outbuf, "pwm_ctl->rctl->rctl_resp_buf .......: %s",
		pwm_ctl->rctl->rctl_resp_buf );
	soft_pwm_log(outbuf, LOG_DUMP);
	sprintf(outbuf, "pwm_ctl->rctl->rctl_errno ..........: %d",
		pwm_ctl->rctl->rctl_errno );
	soft_pwm_log(outbuf, LOG_DUMP);
	sprintf(outbuf, "pwm_ctl->rctl->p_rctl_logbuffer ....: %x",
		(unsigned int) pwm_ctl->rctl->p_rctl_logbuffer );
	soft_pwm_log(outbuf, LOG_DUMP);
	sprintf(outbuf, "pwm_ctl->rctl->rctl_logbuffer ......: %s",
		pwm_ctl->rctl->rctl_logbuffer );
	soft_pwm_log(outbuf, LOG_DUMP);

	sprintf(outbuf, "pwm_ctl->misc_opt->tinfo ..........: %0X",
		pwm_ctl->misc_opt->tinfo ? (unsigned int) pwm_ctl->misc_opt->tinfo : 
		( unsigned int) NULL);
	soft_pwm_log(outbuf, LOG_DUMP);
	sprintf(outbuf, "pwm_ctl->misc_opt->attr ...........: %ld",
		pwm_ctl->misc_opt->attr.__align );
	soft_pwm_log(outbuf, LOG_DUMP);
	sprintf(outbuf, "pwm_ctl->misc_opt->stack_size .....: %0x",
		pwm_ctl->misc_opt->stack_size );
	soft_pwm_log(outbuf, LOG_DUMP);
	sprintf(outbuf, "pwm_ctl->misc_opt->pwm_r_ctl_status: %0x",
		pwm_ctl->misc_opt->pwm_r_ctl_status );
	soft_pwm_log(outbuf, LOG_DUMP);
	sprintf(outbuf, "pwm_ctl->misc_opt->pwm_g_ctl_status: %0x",
		pwm_ctl->misc_opt->pwm_g_ctl_status );
	soft_pwm_log(outbuf, LOG_DUMP);
	sprintf(outbuf, "pwm_ctl->misc_opt->pwm_b_ctl_status: %0x",
		pwm_ctl->misc_opt->pwm_b_ctl_status );
	soft_pwm_log(outbuf, LOG_DUMP);
	sprintf(outbuf, "pwm_ctl->misc_opt->pwm_tcp_ctl_status : %0x",
		pwm_ctl->misc_opt->pwm_tcp_ctl_status );
	soft_pwm_log(outbuf, LOG_DUMP);
	sprintf(outbuf, "pwm_ctl->misc_opt->board_revision ..: %d",
		pwm_ctl->misc_opt->board_revision );
	soft_pwm_log(outbuf, LOG_DUMP);
	sprintf(outbuf, "pwm_ctl->misc_opt->log_facility ....: %d",
		pwm_ctl->misc_opt->log_facility );
	soft_pwm_log(outbuf, LOG_DUMP);
	sprintf(outbuf, "pwm_ctl->misc_opt->log_options .....: %d",
		pwm_ctl->misc_opt->log_options );
	soft_pwm_log(outbuf, LOG_DUMP);
	sprintf(outbuf, "pwm_ctl->misc_opt->daemon_name .....: %s",
		pwm_ctl->misc_opt->daemon_name );
	soft_pwm_log(outbuf, LOG_DUMP);
	sprintf(outbuf, "pwm_ctl->misc_opt->daemon_pid ......: %d",
		pwm_ctl->misc_opt->daemon_pid );
	soft_pwm_log(outbuf, LOG_DUMP);
	sprintf(outbuf, "pwm_ctl->misc_opt->call_time .......: %ld:%ld",
		pwm_ctl->misc_opt->call_time.tv_sec,
		pwm_ctl->misc_opt->call_time.tv_nsec );
	soft_pwm_log(outbuf, LOG_DUMP);
	sprintf(outbuf, "pwm_ctl->misc_opt->start_time ......: %ld:%ld",
		pwm_ctl->misc_opt->start_time.tv_sec,
		pwm_ctl->misc_opt->start_time.tv_nsec );

	soft_pwm_log(outbuf, LOG_DUMP);
	sprintf(outbuf, "pwm_ctl->misc_opt->end_time ........: %ld:%ld",
		pwm_ctl->misc_opt->end_time.tv_sec,
		pwm_ctl->misc_opt->end_time.tv_nsec );
	soft_pwm_log(outbuf, LOG_DUMP);
	sprintf(outbuf, "pwm_ctl->misc_opt->exit_code .......: %d",
		pwm_ctl->misc_opt->exit_code );
	soft_pwm_log(outbuf, LOG_DUMP);
	sprintf(outbuf, "pwm_ctl->misc_opt->last_error ......: %d",
		pwm_ctl->misc_opt->last_error );
	soft_pwm_log(outbuf, LOG_DUMP);
	sprintf(outbuf, "pwm_ctl->misc_opt->verbose_level ...: %d",
		pwm_ctl->misc_opt->verbose_level );
	soft_pwm_log(outbuf, LOG_DUMP);
	sprintf(outbuf, "pwm_ctl->misc_opt->pid_file_name ...: %s",
		pwm_ctl->misc_opt->pid_file_name );
	soft_pwm_log(outbuf, LOG_DUMP);
	sprintf(outbuf, "pwm_ctl->misc_opt->p_log ...........: %x",
		(unsigned int) pwm_ctl->misc_opt->p_log );
	soft_pwm_log(outbuf, LOG_DUMP);
	sprintf(outbuf, "pwm_ctl->misc_opt->logbuffer .......: %s",
		pwm_ctl->misc_opt->logbuffer );
	soft_pwm_log(outbuf, LOG_DUMP);
	sprintf(outbuf, "pwm_ctl->misc_opt->p_cfg ...........: %x",
		(unsigned int) pwm_ctl->misc_opt->p_cfg );
	soft_pwm_log(outbuf, LOG_DUMP);
	sprintf(outbuf, "pwm_ctl->misc_opt->config_file_name : %s",
		pwm_ctl->misc_opt->config_file_name );
	soft_pwm_log(outbuf, LOG_DUMP);

}

/* ---------------------------------------------------------------------------------
 | struct soft_pwm_options_t *soft_pwm_init(void)
 |
 | initialize a control structure for the program that contains all parameters
 -----------------------------------------------------------------------------------
*/

struct soft_pwm_options_t *soft_pwm_init(void)
{
	struct soft_pwm_options_t *retval = NULL;

	if( (retval = (struct soft_pwm_options_t *) 
		malloc(sizeof(struct soft_pwm_options_t))) != NULL )
	{
		if( (retval->r_channel = (struct pwm_thread_data_t*) 
			malloc(sizeof(struct pwm_thread_data_t))) != NULL )
		{
			if( (retval->g_channel = (struct pwm_thread_data_t*) 
				malloc(sizeof(struct pwm_thread_data_t))) != NULL )
			{
				if( (retval->b_channel = (struct pwm_thread_data_t*) 
					malloc(sizeof(struct pwm_thread_data_t))) != NULL )
				{
					if( (retval->rctl = (struct pwm_server_data_t*) 
						malloc(sizeof(struct pwm_server_data_t))) != NULL )
					{
						if( (retval->misc_opt = (struct pwm_general_data_t*) 
							malloc(sizeof(struct pwm_general_data_t))) == NULL )
						{
							retval = soft_pwm_release( retval );
						}
					}
					else
					{
						retval = soft_pwm_release( retval );
					}
				}
				else
				{
					retval = soft_pwm_release( retval );
				}
			}
			else
			{
				retval = soft_pwm_release( retval );
			}
		}
		else
		{
			retval = soft_pwm_release( retval );
		}
	}

	return retval;
}

/* ---------------------------------------------------------------------------------
 | int pin2bcm( int pin )
 |
 | map physical pin to broadcom pin for access
 -----------------------------------------------------------------------------------
*/

int pin2bcm( int pin )
{
	int found = 0;
	int i;

struct phys_bcm_t pins[] = {
	{  3, 2 },
	{  5, 3 },
	{  7, 4 },
	{ 11,17 },
	{ 13,27 },
	{ 15,22 },
	{ 19,10 },
	{ 21, 9 },
	{ 23,11 },
	{  8,13 },
	{ 10,15 },
	{ 12,18 },
	{ 16,23 },
	{ 18,24 },
	{ 22,25 },
	{ 24, 8 },
	{ 26, 7 },
	{  0, 0 } 
};


	for( i = 0; i < sizeof(pins) && found == 0; i++)
	{
		if( pin == pins[i].phys_pin )
		{
			found = (int) pins[i].bcm_pin;
		}

	}

	return( found );
}

/* ---------------------------------------------------------------------------------
 | int soft_pwm_create_thread( struct soft_pwm_options_t *pwm_ctl,
 |	int ThreadId )
 | create and initialize a thread with the given threadid
 -----------------------------------------------------------------------------------
*/

int soft_pwm_create_thread( struct soft_pwm_options_t *pwm_ctl, int ThreadId )
{

	int returnval;

	pwm_ctl->misc_opt->tinfo[ThreadId].thread_num = ThreadId + 1;
	pwm_ctl->misc_opt->tinfo[ThreadId].argv_string = "";


	switch( ThreadId )
	{
		case SOFTPWM_R_THREAD:
			pwm_ctl->misc_opt->pwm_r_ctl_status = SOFTPWM_THREAD_INIT;
			returnval = pthread_create(
				&pwm_ctl->misc_opt->tinfo[ThreadId].thread_id, 
				&pwm_ctl->misc_opt->attr, &run_r_channel_thread,
				&pwm_ctl->misc_opt->tinfo[ThreadId]);
			break;
		case SOFTPWM_G_THREAD:
			pwm_ctl->misc_opt->pwm_g_ctl_status = SOFTPWM_THREAD_INIT;
			returnval = pthread_create(
				&pwm_ctl->misc_opt->tinfo[ThreadId].thread_id, 
				&pwm_ctl->misc_opt->attr, &run_g_channel_thread,
				&pwm_ctl->misc_opt->tinfo[ThreadId]);
			break;
		case SOFTPWM_B_THREAD:
			pwm_ctl->misc_opt->pwm_b_ctl_status = SOFTPWM_THREAD_INIT;
			returnval = pthread_create(
				&pwm_ctl->misc_opt->tinfo[ThreadId].thread_id, 
				&pwm_ctl->misc_opt->attr, &run_b_channel_thread,
				&pwm_ctl->misc_opt->tinfo[ThreadId]);
			break;
		case SOFTPWM_TCP_THREAD:
			pwm_ctl->misc_opt->pwm_tcp_ctl_status = SOFTPWM_THREAD_INIT;
			returnval = pthread_create(
				&pwm_ctl->misc_opt->tinfo[ThreadId].thread_id, 
				&pwm_ctl->misc_opt->attr, &run_tcp_thread,
				&pwm_ctl->misc_opt->tinfo[ThreadId]);
			break;
		default:
			returnval = -1;
	}

	return( returnval );
}

/* ---------------------------------------------------------------------------------
 | void soft_pwm_wait_threads( struct soft_pwm_options_t *pwm_ctl )
 | wait for threads are finished
 -----------------------------------------------------------------------------------
*/

void soft_pwm_wait_threads( struct soft_pwm_options_t *pwm_ctl )
{
	void *r_res;
	void *g_res;
	void *b_res;
	void *tcp_res;

	if( pwm_ctl->misc_opt->tinfo[SOFTPWM_R_THREAD].thread_id >= 0 )
	{
		pthread_join(pwm_ctl->misc_opt->tinfo[SOFTPWM_R_THREAD].thread_id, &r_res);
	}

	if( pwm_ctl->misc_opt->tinfo[SOFTPWM_G_THREAD].thread_id >= 0 )
	{
		pthread_join(pwm_ctl->misc_opt->tinfo[SOFTPWM_G_THREAD].thread_id, &g_res);
	}

	if( pwm_ctl->misc_opt->tinfo[SOFTPWM_B_THREAD].thread_id >= 0 )
	{
		pthread_join(pwm_ctl->misc_opt->tinfo[SOFTPWM_B_THREAD].thread_id, &b_res);
	}

	if( pwm_ctl->misc_opt->tinfo[SOFTPWM_TCP_THREAD].thread_id >= 0 )
	{
		pthread_join(pwm_ctl->misc_opt->tinfo[SOFTPWM_TCP_THREAD].thread_id, &tcp_res);
	}

}

/* ---------------------------------------------------------------------------------
 | int soft_pwm_cancel_thread( struct soft_pwm_options_t *pwm_ctl,
 |	int thread_num )
 | cancel a thread
 -----------------------------------------------------------------------------------
*/

int soft_pwm_cancel_thread( struct soft_pwm_options_t *pwm_ctl, int thread_num )
{
	return pthread_cancel(pwm_ctl->misc_opt->tinfo[thread_num].thread_id);
}

/* ---------------------------------------------------------------------------------
 | Signal handling stuff
 -----------------------------------------------------------------------------------
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

static void catch_SIGUSR1(int val)
{
	return;
}


/* ---------------------------------------------------------------------------------
 | int soft_pwm_daemonize( struct soft_pwm_options_t *pwm_ctl )
 | make program a daemon
 -----------------------------------------------------------------------------------
*/

int soft_pwm_daemonize( struct soft_pwm_options_t *pwm_ctl )
{
	int i;
	pid_t pid;

	if ((pid = fork ()) != 0)
	{
		if( pid < 0 )
		{
			return( errno );
		}
		else
		{
			sprintf( pwm_ctl->misc_opt->logbuffer, "exit fork %d, ->%d", errno, pid);
			soft_pwm_log( pwm_ctl->misc_opt->logbuffer, 1 );
			exit( 0 );
		}
	}

	if (setsid() < 0)
	{
		return( errno );
	}

	handle_signal (SIGHUP, SIG_IGN);

	if ((pid = fork ()) != 0)
	{
		if( pid < 0 )
		{
			return( errno );
		}
		else
		{
			sprintf( pwm_ctl->misc_opt->logbuffer, "exit fork(2) %d", errno);
			soft_pwm_log( pwm_ctl->misc_opt->logbuffer, 1 );
			exit( 0 );
		}
	}


	chdir ("/");
	umask (0);
	for (i = sysconf (_SC_OPEN_MAX); i > 0; i--)
		close (i);

	pwm_ctl->misc_opt->daemon_pid = getpid();

	openlog ( pwm_ctl->misc_opt->daemon_name, 
		pwm_ctl->misc_opt->log_options, pwm_ctl->misc_opt->log_facility );

	handle_signal (SIGUSR1, catch_SIGUSR1);

	clock_gettime( CLOCK_REALTIME, &pwm_ctl->misc_opt->start_time );
}

/*
 ************************************* thread stuff **************************************
*/

/* ---------------------------------------------------------------------------------
 | startup the control thread for red, green or blue channel
 -----------------------------------------------------------------------------------
*/

static void *run_r_channel_thread()
{
	void *return_value = "NULL";
	struct soft_pwm_options_t *pwm_ctl;

	pwm_ctl = global_pwm_ctl;
	pwm_ctl->misc_opt->pwm_r_ctl_status = SOFTPWM_THREAD_RUNNING;
	setup_r_pin( pwm_ctl );

	return( run_rgb_channel_thread(SOFTPWM_R_THREAD, pwm_ctl) );
}


/* ---------------------------------------------------------------------------------
 | exit the control thread for red, green or blue channel
 -----------------------------------------------------------------------------------
*/

static void *exit_r_channel_thread( int exit_code )
{
	struct soft_pwm_options_t *pwm_ctl;

	pwm_ctl = global_pwm_ctl;
	pwm_ctl->misc_opt->pwm_r_ctl_status = SOFTPWM_THREAD_FINISHED;

	return( exit_rgb_channel_thread(SOFTPWM_R_THREAD, exit_code, pwm_ctl) );
}

/* ---------------------------------------------------------------------------------
 | startup the control thread for red, green or blue channel
 -----------------------------------------------------------------------------------
*/

static void *run_g_channel_thread()
{
	void *return_value = "NULL";
	struct soft_pwm_options_t *pwm_ctl;

	pwm_ctl = global_pwm_ctl;
	pwm_ctl->misc_opt->pwm_g_ctl_status = SOFTPWM_THREAD_RUNNING;
	setup_g_pin( pwm_ctl );

	return( run_rgb_channel_thread(SOFTPWM_G_THREAD, pwm_ctl) );
}


/* ---------------------------------------------------------------------------------
 | exit the control thread for red, green or blue channel
 -----------------------------------------------------------------------------------
*/

static void *exit_g_channel_thread( int exit_code )
{
	struct soft_pwm_options_t *pwm_ctl;

	pwm_ctl = global_pwm_ctl;
	pwm_ctl->misc_opt->pwm_g_ctl_status = SOFTPWM_THREAD_FINISHED;

	return( exit_rgb_channel_thread(SOFTPWM_G_THREAD, exit_code, pwm_ctl) );
}

/* ---------------------------------------------------------------------------------
 | startup the control thread for red, green or blue channel
 -----------------------------------------------------------------------------------
*/

static void *run_b_channel_thread()
{
	void *return_value = "NULL";
	struct soft_pwm_options_t *pwm_ctl;

	pwm_ctl = global_pwm_ctl;
	pwm_ctl->misc_opt->pwm_b_ctl_status = SOFTPWM_THREAD_RUNNING;
	setup_b_pin( pwm_ctl );

	return( run_rgb_channel_thread(SOFTPWM_B_THREAD, pwm_ctl) );
}


/* ---------------------------------------------------------------------------------
 | exit the control thread for red, green or blue channel
 -----------------------------------------------------------------------------------
*/

static void *exit_b_channel_thread( int exit_code )
{
	struct soft_pwm_options_t *pwm_ctl;

	pwm_ctl = global_pwm_ctl;
	pwm_ctl->misc_opt->pwm_b_ctl_status = SOFTPWM_THREAD_FINISHED;

	return( exit_rgb_channel_thread(SOFTPWM_B_THREAD, exit_code, pwm_ctl) );
}

int softpwm_initialize( struct soft_pwm_options_t *pwm_ctl )
{
	int failed = 0;

	failed = gpioInitialise();
	return( failed );
}

int softpwm_terminate( struct soft_pwm_options_t *pwm_ctl )
{
	int failed = 0;

	gpioPWM( pwm_ctl->r_channel->pin, 0);
	gpioPWM( pwm_ctl->g_channel->pin, 0);
	gpioPWM( pwm_ctl->b_channel->pin, 0);

	gpioWrite( pwm_ctl->r_channel->pin, 0);
	gpioWrite( pwm_ctl->g_channel->pin, 0);
	gpioWrite( pwm_ctl->b_channel->pin, 0);

	gpioTerminate();
	return( failed );
}

int setup_r_pin( struct soft_pwm_options_t *pwm_ctl )
{
	int failed = 0;
	
	failed = gpioSetMode( pwm_ctl->r_channel->pin, PI_OUTPUT );
	return( failed );
}

int setup_g_pin( struct soft_pwm_options_t *pwm_ctl )
{
	int failed = 0;
	
	failed = gpioSetMode( pwm_ctl->g_channel->pin, PI_OUTPUT );
	return( failed );
}

int setup_b_pin( struct soft_pwm_options_t *pwm_ctl )
{
	int failed = 0;
	
	failed = gpioSetMode( pwm_ctl->b_channel->pin, PI_OUTPUT );
	return( failed );
}

int reinit_r_pin( struct soft_pwm_options_t *pwm_ctl )
{
	int failed = 0;

	failed = gpioSetPWMfrequency ( pwm_ctl->r_channel->pin, 
		(pwm_ctl->r_channel->frequency * pwm_ctl->r_channel->f_multi) );

	failed = gpioPWM( pwm_ctl->r_channel->pin, 
		pwm_ctl->r_channel->intensity);

	if( pwm_ctl->r_channel->frequency == 0 )
	{
		gpioWrite( pwm_ctl->r_channel->pin, 0);
	}

	return( failed );
}

int reinit_g_pin( struct soft_pwm_options_t *pwm_ctl )
{
	int failed = 0;
	
	failed = gpioSetPWMfrequency ( pwm_ctl->g_channel->pin, 
		(pwm_ctl->g_channel->frequency * pwm_ctl->g_channel->f_multi) );

	failed = gpioPWM( pwm_ctl->g_channel->pin, 
		pwm_ctl->g_channel->intensity);

	if( pwm_ctl->g_channel->frequency == 0 )
	{
		gpioWrite( pwm_ctl->g_channel->pin, 0);
	}

	return( failed );
}

int reinit_b_pin( struct soft_pwm_options_t *pwm_ctl )
{
	int failed = 0;
	
	failed = gpioSetPWMfrequency ( pwm_ctl->b_channel->pin, 
		(pwm_ctl->b_channel->frequency * pwm_ctl->b_channel->f_multi) );

	failed = gpioPWM( pwm_ctl->b_channel->pin, 
		pwm_ctl->b_channel->intensity);

	if( pwm_ctl->b_channel->frequency == 0 )
	{
		gpioWrite( pwm_ctl->b_channel->pin, 0);
	}

	return( failed );
}

/* ---------------------------------------------------------------------------------
 | startup the control thread for red, green or blue channel
 -----------------------------------------------------------------------------------
*/

static void *run_rgb_channel_thread(int threadId, struct soft_pwm_options_t *pwm_ctl )
{
	void *return_value = "NULL";

	static int last_r_frequency;
	static int last_g_frequency;
	static int last_b_frequency;
	static int last_r_intensity;
	static int last_g_intensity;
	static int last_b_intensity;
	static int last_r_f_multi;
	static int last_g_f_multi;
	static int last_b_f_multi;
	struct timespec t_req;
	struct timespec t_rem;

	last_r_frequency = pwm_ctl->r_channel->frequency;
	last_g_frequency = pwm_ctl->g_channel->frequency;
	last_b_frequency = pwm_ctl->b_channel->frequency;

	last_r_intensity = pwm_ctl->r_channel->intensity;
	last_g_intensity = pwm_ctl->g_channel->intensity;
	last_b_intensity = pwm_ctl->b_channel->intensity;

	last_r_f_multi = pwm_ctl->r_channel->f_multi;
	last_g_f_multi = pwm_ctl->g_channel->f_multi;
	last_b_f_multi = pwm_ctl->b_channel->f_multi;

	do
	{
		t_req.tv_sec = 0;		/* seconds */
		t_req.tv_nsec = 499999999;	/* nanoseconds */

		switch( threadId )
		{
			case SOFTPWM_R_THREAD:
				if( last_r_frequency != pwm_ctl->r_channel->frequency )
				{
					reinit_r_pin( pwm_ctl );
					last_r_frequency = pwm_ctl->r_channel->frequency;
				}
				if( last_r_intensity != pwm_ctl->r_channel->intensity )
				{
					reinit_r_pin( pwm_ctl );
					last_r_intensity = pwm_ctl->r_channel->intensity;
				}
				if( last_r_f_multi != pwm_ctl->r_channel->f_multi )
				{
					reinit_r_pin( pwm_ctl );
					last_r_f_multi = pwm_ctl->r_channel->f_multi;
				}
				break;
			case SOFTPWM_G_THREAD:
				if( last_g_frequency != pwm_ctl->g_channel->frequency )
				{
					reinit_g_pin( pwm_ctl );
					last_g_frequency = pwm_ctl->g_channel->frequency;
				}
				if( last_g_intensity != pwm_ctl->g_channel->intensity )
				{
					reinit_g_pin( pwm_ctl );
					last_g_intensity = pwm_ctl->g_channel->intensity;
				}
				if( last_g_f_multi != pwm_ctl->g_channel->f_multi )
				{
					reinit_g_pin( pwm_ctl );
					last_g_f_multi = pwm_ctl->g_channel->f_multi;
				}
				break;
			case SOFTPWM_B_THREAD:
				if( last_b_frequency != pwm_ctl->b_channel->frequency )
				{
					reinit_b_pin( pwm_ctl );
					last_b_frequency = pwm_ctl->b_channel->frequency;
				}
				if( last_b_intensity != pwm_ctl->b_channel->intensity )
				{
					reinit_b_pin( pwm_ctl );
					last_b_intensity = pwm_ctl->b_channel->intensity;
				}
				if( last_b_f_multi != pwm_ctl->b_channel->f_multi )
				{
					reinit_b_pin( pwm_ctl );
					last_b_f_multi = pwm_ctl->b_channel->f_multi;
				}
				break;
		}

		nanosleep( &t_req, &t_rem );

	} while( (pwm_ctl->misc_opt->pwm_r_ctl_status != SOFTPWM_THREAD_FINISHED) &&
		 (pwm_ctl->misc_opt->pwm_r_ctl_status != SOFTPWM_THREAD_FAILED) &&
		 (pwm_ctl->misc_opt->pwm_g_ctl_status != SOFTPWM_THREAD_FINISHED) &&
		 (pwm_ctl->misc_opt->pwm_g_ctl_status != SOFTPWM_THREAD_FAILED) &&
		 (pwm_ctl->misc_opt->pwm_b_ctl_status != SOFTPWM_THREAD_FINISHED) &&
		 (pwm_ctl->misc_opt->pwm_b_ctl_status != SOFTPWM_THREAD_FAILED) &&
		 (pwm_ctl->misc_opt->pwm_tcp_ctl_status != SOFTPWM_THREAD_FINISHED) &&
		 (pwm_ctl->misc_opt->pwm_tcp_ctl_status != SOFTPWM_THREAD_FAILED) );

	return( return_value );
}


/* ---------------------------------------------------------------------------------
 | exit the control thread for red, green or blue channel
 -----------------------------------------------------------------------------------
*/

static void *exit_rgb_channel_thread(int threadId, int exit_code, struct soft_pwm_options_t *pwm_ctl )
{
	switch( threadId )
	{
		case SOFTPWM_R_THREAD:
			break;
		case SOFTPWM_G_THREAD:
			break;
		case SOFTPWM_B_THREAD:
			break;
	}

	return( (void*) "NULL" );
// SOFTPWM_THREAD_FAILED;
}




void rctl_prompt( struct soft_pwm_options_t *pwm_ctl )
{

	bzero(pwm_ctl->rctl->p_rctl_recv_buf, 
		sizeof(pwm_ctl->rctl->rctl_recv_buf) );

	sprintf( pwm_ctl->rctl->p_rctl_resp_buf, "%s > ", pwm_ctl->misc_opt->daemon_name );

	write( pwm_ctl->rctl->rctl_cmd_interface, 
		pwm_ctl->rctl->p_rctl_resp_buf, 
		strlen(pwm_ctl->rctl->rctl_resp_buf) );

}


/* ---------------------------------------------------------------------------------
 | static void *run_tcp_thread()
 | startup the control thread for remote control over tcp
 -----------------------------------------------------------------------------------
*/

static void *run_tcp_thread()
{
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

#ifdef NODEF
		if( pwm_ctl->misc_opt->pwm_r_ctl_status != SOFTPWM_THREAD_RUNNING &&
		    pwm_ctl->misc_opt->pwm_g_ctl_status != SOFTPWM_THREAD_RUNNING &&
		    pwm_ctl->misc_opt->pwm_b_ctl_status != SOFTPWM_THREAD_RUNNING &&
		    pwm_ctl->misc_opt->pwm_tcp_ctl_status != SOFTPWM_THREAD_RUNNING )
      		{
			break;
		}
#endif /* NODEF */

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

	return( return_value );
}

/* ---------------------------------------------------------------------------------
 | int do_tcp_command( struct soft_pwm_options_t *pwm_ctl )
 | startup the control thread for remote control over tcp
 -----------------------------------------------------------------------------------
*/

void compact_input( unsigned char *inp_str )
{
	int src, tgt;

	if( inp_str != NULL )
	{
		for( src = 0, tgt = 0; src < strlen(inp_str); src++ )
		{
			if( !isblank(inp_str[src] ) &&
				inp_str[src] != '\n' &&
				inp_str[src] != '\r' )
			{
				inp_str[tgt] = inp_str[src];
				tgt++;
			}
		}
		inp_str[tgt] = '\0';
	}
}





int scan_tcp_command( struct soft_pwm_options_t *pwm_ctl )
{
	int i, cmd_found;

	compact_input( pwm_ctl->rctl->rctl_recv_buf );

	for( i = 0, cmd_found=0; !cmd_found && cmd[i].shortcut != NULL; i++ )
	{
		if(strlen( pwm_ctl->rctl->rctl_recv_buf ) == 1 )
		{
			write( pwm_ctl->rctl->rctl_cmd_interface, 
				pwm_ctl->rctl->p_rctl_resp_buf, 
				strlen(pwm_ctl->rctl->rctl_resp_buf) );

			switch( pwm_ctl->rctl->rctl_recv_buf[0] )
			{
				case 'q':
				case 'Q':
					cmd_found = SOFTPWM_TCP_CMD_CLOSE;
					break;
				case 'x':
				case 'X':
					cmd_found = SOFTPWM_TCP_CMD_EXIT;
					break;
				case '?':
					cmd_found = SOFTPWM_TCP_CMD_HELP;
					break;
				case 'r':
					cmd_found = SOFTPWM_TCP_CMD_GET_RINTENSITY;
					break;
				case 'g':
					cmd_found = SOFTPWM_TCP_CMD_GET_GINTENSITY;
					break;
				case 'b':
					cmd_found = SOFTPWM_TCP_CMD_GET_BINTENSITY;
					break;
				case 'R':
					cmd_found = SOFTPWM_TCP_CMD_GET_RFREQ;
					break;
				case 'G':
					cmd_found = SOFTPWM_TCP_CMD_GET_GFREQ;
					break;
				case 'B':
					cmd_found = SOFTPWM_TCP_CMD_GET_BFREQ;
					break;
				case 'i':
				case 'I':
					cmd_found = SOFTPWM_TCP_CMD_GET_INCR;
					break;
				case 'd':
				case 'D':
					cmd_found = SOFTPWM_TCP_CMD_GET_DECR;
					break;
				case 'a':
				case 'A':
					cmd_found = SOFTPWM_TCP_CMD_GET_ALL;
					break;
				default:
					break;
			}
		}
		else
		{

			if( (strncmp( cmd[i].shortcut, pwm_ctl->rctl->rctl_recv_buf, 
				strlen( cmd[i].shortcut ) ) == 0) ||
				(strncmp( cmd[i].command, pwm_ctl->rctl->rctl_recv_buf, 
				strlen( cmd[i].command ) ) == 0) )
			{
				cmd_found = cmd[i].cmd_number;

				sprintf(pwm_ctl->rctl->p_rctl_resp_buf, 
					"found %s [%s] IDX = %d \n",
					cmd[i].command, cmd[i].shortcut, cmd[i].cmd_number );

				write( pwm_ctl->rctl->rctl_cmd_interface, 
					pwm_ctl->rctl->p_rctl_resp_buf, 
					strlen(pwm_ctl->rctl->rctl_resp_buf) );
			}
		}
	}

	return(cmd_found);
}

void rctl_help( struct soft_pwm_options_t *pwm_ctl )
{
	int i;

	for( i = 0; cmd[i].shortcut != NULL; i++ )
	{
		if( strlen(cmd[i].shortcut) == 1 )
		{
			sprintf(pwm_ctl->rctl->p_rctl_resp_buf, 
				"%12.12s [%s]  %s\n",
				cmd[i].command, cmd[i].shortcut, 
				cmd[i].description);
		}
		else
		{
			sprintf(pwm_ctl->rctl->p_rctl_resp_buf, 
				"%12.12s [%2s] %s\n",
				cmd[i].command, cmd[i].shortcut, 
				cmd[i].description);
		}

		write( pwm_ctl->rctl->rctl_cmd_interface, 
			pwm_ctl->rctl->p_rctl_resp_buf, 
			strlen(pwm_ctl->rctl->rctl_resp_buf) );

	}
}

int set_rfrequency( struct num_val_t value, struct soft_pwm_options_t *pwm_ctl, int multi )
{
	int failed = 0;

	pwm_ctl->r_channel->frequency = value.val.i;
	pwm_ctl->r_channel->f_multi = multi;
	sprintf(pwm_ctl->rctl->p_rctl_resp_buf,"set R-frequency, new value is %d[%d]\n", 
		pwm_ctl->r_channel->frequency, pwm_ctl->r_channel->f_multi );
	write( pwm_ctl->rctl->rctl_cmd_interface, pwm_ctl->rctl->p_rctl_resp_buf, 
		strlen(pwm_ctl->rctl->rctl_resp_buf) );

	return(failed);
}

int set_gfrequency( struct num_val_t value, struct soft_pwm_options_t *pwm_ctl, int multi )
{
	int failed = 0;

	pwm_ctl->g_channel->frequency = value.val.i;
	pwm_ctl->g_channel->f_multi = multi;
	sprintf(pwm_ctl->rctl->p_rctl_resp_buf,"set G-frequency, new value is %d[%d]\n", 
		pwm_ctl->g_channel->frequency, pwm_ctl->g_channel->f_multi );
	write( pwm_ctl->rctl->rctl_cmd_interface, pwm_ctl->rctl->p_rctl_resp_buf, 
		strlen(pwm_ctl->rctl->rctl_resp_buf) );

	return(failed);
}

int set_bfrequency( struct num_val_t value, struct soft_pwm_options_t *pwm_ctl, int multi )
{
	int failed = 0;

	pwm_ctl->b_channel->frequency = value.val.i;
	pwm_ctl->b_channel->f_multi = multi;
	sprintf(pwm_ctl->rctl->p_rctl_resp_buf,"set B-frequency, new value is %d[%d]\n", 
		pwm_ctl->b_channel->frequency, pwm_ctl->b_channel->f_multi );
	write( pwm_ctl->rctl->rctl_cmd_interface, pwm_ctl->rctl->p_rctl_resp_buf, 
		strlen(pwm_ctl->rctl->rctl_resp_buf) );

	return(failed);
}

int decr_rfreq( struct soft_pwm_options_t *pwm_ctl )
{
	pwm_ctl->r_channel->frequency -= pwm_ctl->misc_opt->dec_val;
	if( pwm_ctl->r_channel->frequency < 0 )
	{
		pwm_ctl->r_channel->frequency = 0;
	}
	sprintf(pwm_ctl->rctl->p_rctl_resp_buf, "dec R frequency ok, new value is %d\n", 
		pwm_ctl->r_channel->frequency);
	write( pwm_ctl->rctl->rctl_cmd_interface, pwm_ctl->rctl->p_rctl_resp_buf, 
		strlen(pwm_ctl->rctl->rctl_resp_buf) );
}

int decr_gfreq( struct soft_pwm_options_t *pwm_ctl )
{
	pwm_ctl->g_channel->frequency -= pwm_ctl->misc_opt->dec_val;
	if( pwm_ctl->g_channel->frequency < 0 )
	{
		pwm_ctl->g_channel->frequency = 0;
	}
	sprintf(pwm_ctl->rctl->p_rctl_resp_buf, "dec G frequency ok, new value is %d\n", 
		pwm_ctl->g_channel->frequency);
	write( pwm_ctl->rctl->rctl_cmd_interface, pwm_ctl->rctl->p_rctl_resp_buf, 
		strlen(pwm_ctl->rctl->rctl_resp_buf) );
}

int decr_bfreq( struct soft_pwm_options_t *pwm_ctl )
{
	pwm_ctl->b_channel->frequency -= pwm_ctl->misc_opt->dec_val;
	if( pwm_ctl->b_channel->frequency < 0 )
	{
		pwm_ctl->b_channel->frequency = 0;
	}
	sprintf(pwm_ctl->rctl->p_rctl_resp_buf, "dec B frequency ok, new value is %d\n", 
		pwm_ctl->b_channel->frequency);
	write( pwm_ctl->rctl->rctl_cmd_interface, pwm_ctl->rctl->p_rctl_resp_buf, 
		strlen(pwm_ctl->rctl->rctl_resp_buf) );
}

int incr_rfreq( struct soft_pwm_options_t *pwm_ctl )
{
	pwm_ctl->r_channel->frequency += pwm_ctl->misc_opt->inc_val;
	sprintf(pwm_ctl->rctl->p_rctl_resp_buf, "inc R frequency ok, new value is %d\n", 
		pwm_ctl->r_channel->frequency);
	write( pwm_ctl->rctl->rctl_cmd_interface, pwm_ctl->rctl->p_rctl_resp_buf, 
		strlen(pwm_ctl->rctl->rctl_resp_buf) );
}

int incr_gfreq( struct soft_pwm_options_t *pwm_ctl )
{
	pwm_ctl->g_channel->frequency += pwm_ctl->misc_opt->inc_val;
	sprintf(pwm_ctl->rctl->p_rctl_resp_buf, "inc G frequency ok, new value is %d\n", 
		pwm_ctl->g_channel->frequency);
	write( pwm_ctl->rctl->rctl_cmd_interface, pwm_ctl->rctl->p_rctl_resp_buf, 
		strlen(pwm_ctl->rctl->rctl_resp_buf) );
}

int incr_bfreq( struct soft_pwm_options_t *pwm_ctl )
{
	pwm_ctl->b_channel->frequency += pwm_ctl->misc_opt->inc_val;
	sprintf(pwm_ctl->rctl->p_rctl_resp_buf, "inc B frequency ok, new value is %d\n", 
		pwm_ctl->b_channel->frequency);
	write( pwm_ctl->rctl->rctl_cmd_interface, pwm_ctl->rctl->p_rctl_resp_buf, 
		strlen(pwm_ctl->rctl->rctl_resp_buf) );
}

int incr_rchannel( struct soft_pwm_options_t *pwm_ctl )
{
	pwm_ctl->r_channel->intensity++;
	if( pwm_ctl->r_channel->intensity > 255 )
	{
		pwm_ctl->r_channel->intensity = 255;
	}
	sprintf(pwm_ctl->rctl->p_rctl_resp_buf, "incr rchannel ok, new value is %d\n", 
		pwm_ctl->r_channel->intensity);
	write( pwm_ctl->rctl->rctl_cmd_interface, pwm_ctl->rctl->p_rctl_resp_buf, 
		strlen(pwm_ctl->rctl->rctl_resp_buf) );
}

int incr_gchannel( struct soft_pwm_options_t *pwm_ctl )
{
	pwm_ctl->g_channel->intensity += pwm_ctl->misc_opt->inc_val;
	if( pwm_ctl->g_channel->intensity > 255 )
	{
		pwm_ctl->g_channel->intensity = 255;
	}
	sprintf(pwm_ctl->rctl->p_rctl_resp_buf, "incr gchannel ok, new value is %d\n", 
		pwm_ctl->g_channel->intensity);
	write( pwm_ctl->rctl->rctl_cmd_interface, pwm_ctl->rctl->p_rctl_resp_buf, 
		strlen(pwm_ctl->rctl->rctl_resp_buf) );
}

int incr_bchannel( struct soft_pwm_options_t *pwm_ctl )
{
	pwm_ctl->b_channel->intensity += pwm_ctl->misc_opt->inc_val;
	if( pwm_ctl->b_channel->intensity > 255 )
	{
		pwm_ctl->b_channel->intensity = 255;
	}
	sprintf(pwm_ctl->rctl->p_rctl_resp_buf, "incr bchannel ok, new value is %d\n", 
		pwm_ctl->b_channel->intensity);
	write( pwm_ctl->rctl->rctl_cmd_interface, pwm_ctl->rctl->p_rctl_resp_buf, 
		strlen(pwm_ctl->rctl->rctl_resp_buf) );
}


int decr_rchannel( struct soft_pwm_options_t *pwm_ctl )
{
	pwm_ctl->r_channel->intensity -= pwm_ctl->misc_opt->dec_val;
	if( pwm_ctl->r_channel->intensity < 0 )
	{
		pwm_ctl->r_channel->intensity = 0;
	}
	sprintf(pwm_ctl->rctl->p_rctl_resp_buf, "decr rchannel ok, new value is %d\n", 
		pwm_ctl->r_channel->intensity);
	write( pwm_ctl->rctl->rctl_cmd_interface, pwm_ctl->rctl->p_rctl_resp_buf, 
		strlen(pwm_ctl->rctl->rctl_resp_buf) );
}

int decr_gchannel( struct soft_pwm_options_t *pwm_ctl )
{
	pwm_ctl->g_channel->intensity -= pwm_ctl->misc_opt->dec_val;
	if( pwm_ctl->g_channel->intensity < 0 )
	{
		pwm_ctl->g_channel->intensity = 0;
	}
	sprintf(pwm_ctl->rctl->p_rctl_resp_buf, "decr gchannel ok, new value is %d\n", 
		pwm_ctl->g_channel->intensity);
	write( pwm_ctl->rctl->rctl_cmd_interface, pwm_ctl->rctl->p_rctl_resp_buf, 
		strlen(pwm_ctl->rctl->rctl_resp_buf) );
}

int decr_bchannel( struct soft_pwm_options_t *pwm_ctl )
{
	pwm_ctl->b_channel->intensity -= pwm_ctl->misc_opt->dec_val;
	if( pwm_ctl->b_channel->intensity < 0 )
	{
		pwm_ctl->b_channel->intensity = 0;
	}
	sprintf(pwm_ctl->rctl->p_rctl_resp_buf, "decr bchannel ok, new value is %d\n", 
		pwm_ctl->b_channel->intensity);
	write( pwm_ctl->rctl->rctl_cmd_interface, pwm_ctl->rctl->p_rctl_resp_buf, 
		strlen(pwm_ctl->rctl->rctl_resp_buf) );
}

int set_rchannel( struct num_val_t value, struct soft_pwm_options_t *pwm_ctl )
{
	int failed = 0;

	if( value.val.i > 255 )
	{
		value.val.i = 255;
	}
	else
	{
		if( value.val.i < 0 )
		{
			value.val.i = 0;
		}
	}

	pwm_ctl->r_channel->intensity = value.val.i;
	sprintf(pwm_ctl->rctl->p_rctl_resp_buf, "set rchannel ok, new value is %d\n", 
		pwm_ctl->r_channel->intensity);
	write( pwm_ctl->rctl->rctl_cmd_interface, pwm_ctl->rctl->p_rctl_resp_buf, 
		strlen(pwm_ctl->rctl->rctl_resp_buf) );

	return(failed);
}

int set_gchannel( struct num_val_t value, struct soft_pwm_options_t *pwm_ctl )
{
	int failed = 0;

	if( value.val.i > 255 )
	{
		value.val.i = 255;
	}
	else
	{
		if( value.val.i < 0 )
		{
			value.val.i = 0;
		}
	}

	pwm_ctl->g_channel->intensity = value.val.i;
	sprintf(pwm_ctl->rctl->p_rctl_resp_buf, "set gchannel ok, new value is %d\n", 
		pwm_ctl->g_channel->intensity);
	write( pwm_ctl->rctl->rctl_cmd_interface, pwm_ctl->rctl->p_rctl_resp_buf, 
		strlen(pwm_ctl->rctl->rctl_resp_buf) );

	return(failed);
}

int set_bchannel( struct num_val_t value, struct soft_pwm_options_t *pwm_ctl )
{
	int failed = 0;

	if( value.val.i > 255 )
	{
		value.val.i = 255;
	}
	else
	{
		if( value.val.i < 0 )
		{
			value.val.i = 0;
		}
	}

	pwm_ctl->b_channel->intensity = value.val.i;
	sprintf(pwm_ctl->rctl->p_rctl_resp_buf, "set bchannel ok, new value is %d\n", 
		pwm_ctl->b_channel->intensity);
	write( pwm_ctl->rctl->rctl_cmd_interface, pwm_ctl->rctl->p_rctl_resp_buf, 
		strlen(pwm_ctl->rctl->rctl_resp_buf) );

	return(failed);
}


int set_incr( struct num_val_t value, struct soft_pwm_options_t *pwm_ctl )
{
	int failed = 0;

	pwm_ctl->misc_opt->inc_val = value.val.i;
	sprintf(pwm_ctl->rctl->p_rctl_resp_buf, "set inc step, new value is %d\n", value.val.i);
	write( pwm_ctl->rctl->rctl_cmd_interface, pwm_ctl->rctl->p_rctl_resp_buf, 
		strlen(pwm_ctl->rctl->rctl_resp_buf) );

	return(failed);
}

int set_decr( struct num_val_t value, struct soft_pwm_options_t *pwm_ctl )
{
	int failed = 0;

	pwm_ctl->misc_opt->dec_val = value.val.i;
	sprintf(pwm_ctl->rctl->p_rctl_resp_buf, "set dec step, new value is %d\n", value.val.i);
	write( pwm_ctl->rctl->rctl_cmd_interface, pwm_ctl->rctl->p_rctl_resp_buf, 
		strlen(pwm_ctl->rctl->rctl_resp_buf) );

	return(failed);
}


int get_value( unsigned char *buf, struct num_val_t *value )
{
	int error = 0;
	int ival;

	ival = atoi(buf);
	value->val_type = SOFTPWM_VALTYPE_INT;
	value->unit_type = SOFTPWM_UNITTYPE_NONE;
	value->unit_pre = SOFTPWM_UNITPRE_NONE;
	value->val.i = ival;

	return(error);
}


int do_tcp_command( struct soft_pwm_options_t *pwm_ctl )
{
	int cmd;
	struct num_val_t value;
	int failed;
	int i;
	int multi;

	failed = 0;


	switch( (cmd = scan_tcp_command( pwm_ctl )) )
	{
		case SOFTPWM_TCP_CMD_NOT_FOUND:
			break;
		case SOFTPWM_TCP_CMD_CLOSE:
			break;
		case SOFTPWM_TCP_CMD_EXIT:
			break;
		case SOFTPWM_TCP_CMD_HELP:
			rctl_help( pwm_ctl );
			break;
		case SOFTPWM_TCP_CMD_SET_RINTENSITY:
			for( i = 0; pwm_ctl->rctl->rctl_recv_buf[i] &&
				pwm_ctl->rctl->rctl_recv_buf[i] != '=' ; i++ )
				;
			if( pwm_ctl->rctl->rctl_recv_buf[i] == '=' &&
				pwm_ctl->rctl->rctl_recv_buf[i+1] )
			{
				failed=get_value( &pwm_ctl->rctl->rctl_recv_buf[i+1], &value);
				if( failed )
				{
					sprintf(pwm_ctl->rctl->p_rctl_resp_buf,
						"invalid argument for r= !\n");
				}
				else
				{
					failed = set_rchannel( value, pwm_ctl );
				}
			}
			else
			{
				sprintf(pwm_ctl->rctl->p_rctl_resp_buf, "missing argument!\n");
			}
			break;
		case SOFTPWM_TCP_CMD_SET_GINTENSITY:
			for( i = 0; pwm_ctl->rctl->rctl_recv_buf[i] &&
				pwm_ctl->rctl->rctl_recv_buf[i] != '=' ; i++ )
				;
			if( pwm_ctl->rctl->rctl_recv_buf[i] == '=' &&
				pwm_ctl->rctl->rctl_recv_buf[i+1] )
			{
				failed=get_value( &pwm_ctl->rctl->rctl_recv_buf[i+1], &value);
				if( failed )
				{
					sprintf(pwm_ctl->rctl->p_rctl_resp_buf,
						"invalid argument for r= !\n");
				}
				else
				{
					failed = set_gchannel( value, pwm_ctl );
				}
			}
			else
			{
				sprintf(pwm_ctl->rctl->p_rctl_resp_buf, "missing argument!\n");
			}
			break;
		case SOFTPWM_TCP_CMD_SET_BINTENSITY:
			for( i = 0; pwm_ctl->rctl->rctl_recv_buf[i] &&
				pwm_ctl->rctl->rctl_recv_buf[i] != '=' ; i++ )
				;
			if( pwm_ctl->rctl->rctl_recv_buf[i] == '=' &&
				pwm_ctl->rctl->rctl_recv_buf[i+1] )
			{
				failed=get_value( &pwm_ctl->rctl->rctl_recv_buf[i+1], &value);
				if( failed )
				{
					sprintf(pwm_ctl->rctl->p_rctl_resp_buf,
						"invalid argument for r= !\n");
				}
				else
				{
					failed = set_bchannel( value, pwm_ctl );
				}
			}
			else
			{
				sprintf(pwm_ctl->rctl->p_rctl_resp_buf, "missing argument!\n");
			}
			break;
		case SOFTPWM_TCP_CMD_GET_RINTENSITY:
			sprintf(pwm_ctl->rctl->p_rctl_resp_buf, 
				"\ncurrently r-channel intensity is ....: %d\n", 
				pwm_ctl->r_channel->intensity);
			write( pwm_ctl->rctl->rctl_cmd_interface, 
				pwm_ctl->rctl->p_rctl_resp_buf, 
				strlen(pwm_ctl->rctl->rctl_resp_buf) );
			break;
		case SOFTPWM_TCP_CMD_GET_GINTENSITY:
			sprintf(pwm_ctl->rctl->p_rctl_resp_buf, 
				"\ncurrently g-channel intensity is ....: %d\n", 
				pwm_ctl->g_channel->intensity);
			write( pwm_ctl->rctl->rctl_cmd_interface, 
				pwm_ctl->rctl->p_rctl_resp_buf, 
				strlen(pwm_ctl->rctl->rctl_resp_buf) );
			break;
		case SOFTPWM_TCP_CMD_GET_BINTENSITY:
			sprintf(pwm_ctl->rctl->p_rctl_resp_buf, 
				"\ncurrently b-channel intensity is ....: %d\n", 
				pwm_ctl->g_channel->intensity);
			write( pwm_ctl->rctl->rctl_cmd_interface, 
				pwm_ctl->rctl->p_rctl_resp_buf, 
				strlen(pwm_ctl->rctl->rctl_resp_buf) );
			break;
		case SOFTPWM_TCP_CMD_GET_ALL:
			sprintf(pwm_ctl->rctl->p_rctl_resp_buf, 
				"\ncurrent r-frequency is ....: %d\n", 
				pwm_ctl->r_channel->frequency);
			write( pwm_ctl->rctl->rctl_cmd_interface, 
				pwm_ctl->rctl->p_rctl_resp_buf, 
				strlen(pwm_ctl->rctl->rctl_resp_buf) );

			sprintf(pwm_ctl->rctl->p_rctl_resp_buf, 
				"current g-frequency is ....: %d\n", 
				pwm_ctl->g_channel->frequency);
			write( pwm_ctl->rctl->rctl_cmd_interface, 
				pwm_ctl->rctl->p_rctl_resp_buf, 
				strlen(pwm_ctl->rctl->rctl_resp_buf) );

			sprintf(pwm_ctl->rctl->p_rctl_resp_buf, 
				"current b-frequency is ....: %d\n", 
				pwm_ctl->b_channel->frequency);
			write( pwm_ctl->rctl->rctl_cmd_interface, 
				pwm_ctl->rctl->p_rctl_resp_buf, 
				strlen(pwm_ctl->rctl->rctl_resp_buf) );

			sprintf(pwm_ctl->rctl->p_rctl_resp_buf, 
				"current increment value is : %d\n", 
				pwm_ctl->misc_opt->inc_val);
			write( pwm_ctl->rctl->rctl_cmd_interface, 
				pwm_ctl->rctl->p_rctl_resp_buf, 
				strlen(pwm_ctl->rctl->rctl_resp_buf) );

			sprintf(pwm_ctl->rctl->p_rctl_resp_buf, 
				"current decrement value is : %d\n", 
				pwm_ctl->misc_opt->dec_val);
			write( pwm_ctl->rctl->rctl_cmd_interface, 
				pwm_ctl->rctl->p_rctl_resp_buf, 
				strlen(pwm_ctl->rctl->rctl_resp_buf) );

			sprintf(pwm_ctl->rctl->p_rctl_resp_buf, 
				"currently r-channel intensity is ....: %d\n", 
				pwm_ctl->r_channel->intensity);
			write( pwm_ctl->rctl->rctl_cmd_interface, 
				pwm_ctl->rctl->p_rctl_resp_buf, 
				strlen(pwm_ctl->rctl->rctl_resp_buf) );

			sprintf(pwm_ctl->rctl->p_rctl_resp_buf, 
				"currently g-channel intensity is ....: %d\n", 
				pwm_ctl->g_channel->intensity);
			write( pwm_ctl->rctl->rctl_cmd_interface, 
				pwm_ctl->rctl->p_rctl_resp_buf, 
				strlen(pwm_ctl->rctl->rctl_resp_buf) );

			sprintf(pwm_ctl->rctl->p_rctl_resp_buf, 
				"currently b-channel intensity is ....: %d\n", 
				pwm_ctl->b_channel->intensity);
			write( pwm_ctl->rctl->rctl_cmd_interface, 
				pwm_ctl->rctl->p_rctl_resp_buf, 
				strlen(pwm_ctl->rctl->rctl_resp_buf) );

			break;
		case SOFTPWM_TCP_CMD_INCR_RINTENSITY:
			failed = incr_rchannel( pwm_ctl );
			break;
		case SOFTPWM_TCP_CMD_INCR_GINTENSITY:
			failed = incr_gchannel( pwm_ctl );
			break;
		case SOFTPWM_TCP_CMD_INCR_BINTENSITY:
			failed = incr_bchannel( pwm_ctl );
			break;
		case SOFTPWM_TCP_CMD_DECR_RINTENSITY:
			failed = decr_rchannel( pwm_ctl );
			break;
		case SOFTPWM_TCP_CMD_DECR_GINTENSITY:
			failed = decr_gchannel( pwm_ctl );
			break;
		case SOFTPWM_TCP_CMD_DECR_BINTENSITY:
			failed = decr_bchannel( pwm_ctl );
			break;
		case SOFTPWM_TCP_CMD_INCR_RFREQ:
			failed = incr_rfreq( pwm_ctl );
			break;
		case SOFTPWM_TCP_CMD_INCR_GFREQ:
			failed = incr_gfreq( pwm_ctl );
			break;
		case SOFTPWM_TCP_CMD_INCR_BFREQ:
			failed = incr_bfreq( pwm_ctl );
			break;
		case SOFTPWM_TCP_CMD_DECR_RFREQ:
			failed = decr_rfreq( pwm_ctl );
			break;
		case SOFTPWM_TCP_CMD_DECR_GFREQ:
			failed = decr_gfreq( pwm_ctl );
			break;
		case SOFTPWM_TCP_CMD_DECR_BFREQ:
			failed = decr_bfreq( pwm_ctl );
			break;
		case SOFTPWM_TCP_CMD_SET_RFREQ:
			for( i = 0; pwm_ctl->rctl->rctl_recv_buf[i] &&
				pwm_ctl->rctl->rctl_recv_buf[i] != '=' ; i++ )
				;
			if( pwm_ctl->rctl->rctl_recv_buf[i] == '=' &&
				pwm_ctl->rctl->rctl_recv_buf[i+1] )
			{
				failed=get_value( &pwm_ctl->rctl->rctl_recv_buf[i+1], &value);
				i++;
				if( failed )
				{
					sprintf(pwm_ctl->rctl->p_rctl_resp_buf,
						"invalid argument for R= !\n");
				}
				else
				{
					for(; pwm_ctl->rctl->rctl_recv_buf[i] &&
						isdigit(pwm_ctl->rctl->rctl_recv_buf[i]) ; i++ )
						;

					multi = SOFTPWM_KHZ_MULTI;

					if( pwm_ctl->rctl->rctl_recv_buf[i] )
					{

						switch( pwm_ctl->rctl->rctl_recv_buf[i] )
						{

							case 'h':
							case 'H':
								multi = SOFTPWM_HZ_MULTI;
								break;
							case 'k':
							case 'K':
								multi = SOFTPWM_KHZ_MULTI;
								break;
							case 'm':
							case 'M':
								multi = SOFTPWM_MHZ_MULTI;
								break;
						}
					}

					failed = set_rfrequency( value, pwm_ctl, multi );
				}
			}
			else
			{
				sprintf(pwm_ctl->rctl->p_rctl_resp_buf, "missing argument!\n");
			}
			break;
		case SOFTPWM_TCP_CMD_SET_GFREQ:
			for( i = 0; pwm_ctl->rctl->rctl_recv_buf[i] &&
				pwm_ctl->rctl->rctl_recv_buf[i] != '=' ; i++ )
				;
			if( pwm_ctl->rctl->rctl_recv_buf[i] == '=' &&
				pwm_ctl->rctl->rctl_recv_buf[i+1] )
			{
				failed=get_value( &pwm_ctl->rctl->rctl_recv_buf[i+1], &value);
				i++;
				if( failed )
				{
					sprintf(pwm_ctl->rctl->p_rctl_resp_buf,
						"invalid argument for G= !\n");
				}
				else
				{
					for(; pwm_ctl->rctl->rctl_recv_buf[i] &&
						isdigit(pwm_ctl->rctl->rctl_recv_buf[i]) ; i++ )
						;

					multi = SOFTPWM_KHZ_MULTI;

					if( pwm_ctl->rctl->rctl_recv_buf[i] )
					{

						switch( pwm_ctl->rctl->rctl_recv_buf[i] )
						{

							case 'h':
							case 'H':
								multi = SOFTPWM_HZ_MULTI;
								break;
							case 'k':
							case 'K':
								multi = SOFTPWM_KHZ_MULTI;
								break;
							case 'm':
							case 'M':
								multi = SOFTPWM_MHZ_MULTI;
								break;
						}
					}

					failed = set_gfrequency( value, pwm_ctl, multi );
				}
			}
			else
			{
				sprintf(pwm_ctl->rctl->p_rctl_resp_buf, "missing argument!\n");
			}
			break;
		case SOFTPWM_TCP_CMD_SET_BFREQ:
			for( i = 0; pwm_ctl->rctl->rctl_recv_buf[i] &&
				pwm_ctl->rctl->rctl_recv_buf[i] != '=' ; i++ )
				;
			if( pwm_ctl->rctl->rctl_recv_buf[i] == '=' &&
				pwm_ctl->rctl->rctl_recv_buf[i+1] )
			{
				failed=get_value( &pwm_ctl->rctl->rctl_recv_buf[i+1], &value);
				i++;
				if( failed )
				{
					sprintf(pwm_ctl->rctl->p_rctl_resp_buf,
						"invalid argument for B= !\n");
				}
				else
				{
					for(; pwm_ctl->rctl->rctl_recv_buf[i] &&
						isdigit(pwm_ctl->rctl->rctl_recv_buf[i]) ; i++ )
						;

					multi = SOFTPWM_KHZ_MULTI;

					if( pwm_ctl->rctl->rctl_recv_buf[i] )
					{

						switch( pwm_ctl->rctl->rctl_recv_buf[i] )
						{

							case 'h':
							case 'H':
								multi = SOFTPWM_HZ_MULTI;
								break;
							case 'k':
							case 'K':
								multi = SOFTPWM_KHZ_MULTI;
								break;
							case 'm':
							case 'M':
								multi = SOFTPWM_MHZ_MULTI;
								break;
						}
					}

					failed = set_bfrequency( value, pwm_ctl, multi );
				}
			}
			else
			{
				sprintf(pwm_ctl->rctl->p_rctl_resp_buf, "missing argument!\n");
			}
			break;
		case SOFTPWM_TCP_CMD_GET_RFREQ:
			sprintf(pwm_ctl->rctl->p_rctl_resp_buf, 
				"\ncurrently r-frequency is ....: %d\n", 
				pwm_ctl->r_channel->frequency);
			write( pwm_ctl->rctl->rctl_cmd_interface, 
				pwm_ctl->rctl->p_rctl_resp_buf, 
				strlen(pwm_ctl->rctl->rctl_resp_buf) );
			break;
		case SOFTPWM_TCP_CMD_GET_GFREQ:
			sprintf(pwm_ctl->rctl->p_rctl_resp_buf, 
				"\ncurrently g-frequency is ....: %d\n", 
				pwm_ctl->g_channel->frequency);
			write( pwm_ctl->rctl->rctl_cmd_interface, 
				pwm_ctl->rctl->p_rctl_resp_buf, 
				strlen(pwm_ctl->rctl->rctl_resp_buf) );
			break;
		case SOFTPWM_TCP_CMD_GET_BFREQ:
			sprintf(pwm_ctl->rctl->p_rctl_resp_buf, 
				"\ncurrently b-frequency is ....: %d\n", 
				pwm_ctl->b_channel->frequency);
			write( pwm_ctl->rctl->rctl_cmd_interface, 
				pwm_ctl->rctl->p_rctl_resp_buf, 
				strlen(pwm_ctl->rctl->rctl_resp_buf) );
			break;
		case SOFTPWM_TCP_CMD_GET_INCR:
			sprintf(pwm_ctl->rctl->p_rctl_resp_buf, 
				"\ncurrent increment value is : %d\n", 
				pwm_ctl->misc_opt->inc_val);
			write( pwm_ctl->rctl->rctl_cmd_interface, 
				pwm_ctl->rctl->p_rctl_resp_buf, 
				strlen(pwm_ctl->rctl->rctl_resp_buf) );
			break;
		case SOFTPWM_TCP_CMD_SET_INCR:
			for( i = 0; pwm_ctl->rctl->rctl_recv_buf[i] &&
				pwm_ctl->rctl->rctl_recv_buf[i] != '=' ; i++ )
				;
			if( pwm_ctl->rctl->rctl_recv_buf[i] == '=' &&
				pwm_ctl->rctl->rctl_recv_buf[i+1] )
			{
				failed=get_value( &pwm_ctl->rctl->rctl_recv_buf[i+1], &value);
				if( failed )
				{
					sprintf(pwm_ctl->rctl->p_rctl_resp_buf,
						"invalid argument for r= !\n");
				}
				else
				{
					failed = set_incr( value, pwm_ctl );
				}
			}
			else
			{
				sprintf(pwm_ctl->rctl->p_rctl_resp_buf, "missing argument!\n");
			}
			break;
		case SOFTPWM_TCP_CMD_GET_DECR:
			sprintf(pwm_ctl->rctl->p_rctl_resp_buf, 
				"\ncurrent decrement value is : %d\n", 
				pwm_ctl->misc_opt->dec_val);
			write( pwm_ctl->rctl->rctl_cmd_interface, 
				pwm_ctl->rctl->p_rctl_resp_buf, 
				strlen(pwm_ctl->rctl->rctl_resp_buf) );
			break;
		case SOFTPWM_TCP_CMD_SET_DECR:
			for( i = 0; pwm_ctl->rctl->rctl_recv_buf[i] &&
				pwm_ctl->rctl->rctl_recv_buf[i] != '=' ; i++ )
				;
			if( pwm_ctl->rctl->rctl_recv_buf[i] == '=' &&
				pwm_ctl->rctl->rctl_recv_buf[i+1] )
			{
				failed=get_value( &pwm_ctl->rctl->rctl_recv_buf[i+1], &value);
				if( failed )
				{
					sprintf(pwm_ctl->rctl->p_rctl_resp_buf,
						"invalid argument for r= !\n");
				}
				else
				{
					failed = set_decr( value, pwm_ctl );
				}
			}
			else
			{
				sprintf(pwm_ctl->rctl->p_rctl_resp_buf, "missing argument!\n");
			}
			break;
		default:
			break;
	}

	return( cmd );
}


/* ---------------------------------------------------------------------------------
 | static int prepare_server_socket( struct soft_pwm_options_t *pwm_ctl )
 | prepare socket for listen to
 -----------------------------------------------------------------------------------
*/

static int prepare_server_socket( struct soft_pwm_options_t *pwm_ctl )
{
	int on = 1;
	int error_code = 0;

	if((pwm_ctl->rctl->rctl_listener = socket(AF_INET, SOCK_STREAM, 0)) >= 0)
	{
		if(setsockopt(pwm_ctl->rctl->rctl_listener, SOL_SOCKET, SO_REUSEADDR,
			(char *)&on, sizeof(on)) >= 0)
		{
			if(bind(pwm_ctl->rctl->rctl_listener,
				(struct sockaddr *)&pwm_ctl->rctl->rctl_serveraddr, 
				sizeof(pwm_ctl->rctl->rctl_serveraddr)) < 0)
			{
				error_code = errno;
				close(pwm_ctl->rctl->rctl_listener);
			}
		}
		else
		{
			error_code = errno;
			close(pwm_ctl->rctl->rctl_listener);
		}
	}
	else
	{
		error_code = errno;
	}

	return (error_code);
}

/* ---------------------------------------------------------------------------------
 | exit the control thread for remote control over tcp
 -----------------------------------------------------------------------------------
*/

static void *exit_tcp_thread( int exit_code )
{
	struct soft_pwm_options_t *pwm_ctl;

	pwm_ctl = global_pwm_ctl;
	pwm_ctl->misc_opt->pwm_tcp_ctl_status = SOFTPWM_THREAD_FINISHED;
//	pwm_ctl->misc_opt->pwm_tcp_ctl_status = SOFTPWM_THREAD_FAILED;


	return( (void*) "NULL" );
}

/*
 *********************************** end of thread stuff *********************************
*/

/* ---------------------------------------------------------------------------------
 | void soft_pwm_log(unsigned char *outbuf, int logflag)
 | write a log entry depending on loglevel
 -----------------------------------------------------------------------------------
*/

extern void soft_pwm_log(unsigned char *outbuf, int logflag)
{
//	#define LOG_CRITICAL_K	 1
//	#define LOG_ERROR_K	 2
//	#define LOG_WARN_K	 3
//	#define LOG_INFO_K	 4
//	#define LOG_DUMP_K	 5
//	#define LOG_CRITICAL	 6
//	#define LOG_ERROR	 7
//	#define LOG_WARN	 8
//	#define LOG_INFO	 9
//	#define LOG_DUMP	10

syslog(LOG_ERR, "%s\n", outbuf);
	return;
}

/* ---------------------------------------------------------------------------------
 | void read_config(void)
 | read configuration file and set matching parameters
 -----------------------------------------------------------------------------------
*/

void read_config(void)
{
	return;
}


/* ---------------------------------------------------------------------------------
 | main / control loop
 -----------------------------------------------------------------------------------
*/

int main (int argc, char **argv) {
	struct soft_pwm_options_t *pwm_ctl;
	FILE *fd_pid;
	struct timespec call_time;
	int res;

	int i;
	pid_t pid;
	clock_gettime( CLOCK_REALTIME, &call_time );


	if( (pwm_ctl = soft_pwm_init()) != NULL )
	{
		global_pwm_ctl = pwm_ctl;
		clear_pwm_options( pwm_ctl );
		soft_pwm_showargs( pwm_ctl );
		set_pwm_default_options( pwm_ctl );
		get_arguments ( argc, argv, pwm_ctl );
		read_config();

		pwm_ctl->misc_opt->call_time = call_time;

   		res = soft_pwm_daemonize( pwm_ctl );
		sprintf( pwm_ctl->misc_opt->logbuffer, "daemonize returns %d", res);
		soft_pwm_log( pwm_ctl->misc_opt->logbuffer, 1 );

		soft_pwm_showargs( pwm_ctl );

   		if( (fd_pid = fopen(pwm_ctl->misc_opt->pid_file_name, "w+")) != NULL )
   		{
      			fprintf(fd_pid, "%d", pwm_ctl->misc_opt->daemon_pid );
      			fclose( fd_pid );
			errno =0;
   		}
   		else
   		{
			errno =0;
   		}

		sprintf( pwm_ctl->misc_opt->logbuffer, "open pid returns %d", errno);
		soft_pwm_log( pwm_ctl->misc_opt->logbuffer, 1 );

		/* Initialize thread creation attributes */

		if( pthread_attr_init(&pwm_ctl->misc_opt->attr) != 0 )
		{
			soft_pwm_log( "pthread_attr_init failed!", 1 );
		}

		if( pthread_attr_setstacksize(&pwm_ctl->misc_opt->attr, 
			pwm_ctl->misc_opt->stack_size) != 0 )
		{
			soft_pwm_log( "pthread_attr_setstacksize failed!", 1 );
		}

		/* Allocate memory for pthread_create() arguments */

		if( (pwm_ctl->misc_opt->tinfo = calloc(SOFTPWM_NUM_THREADS, 
			sizeof(struct thread_info))) == NULL )
		{
			soft_pwm_log( "calloc failed!", 1 );
		}

		/* Create a control thread for red, green and blue channel */
		/* and create a thread for remote control over TCP         */
		/* The pthread_create() call stores the thread ID into
			corresponding element of tinfo[] */

		res = softpwm_initialize( pwm_ctl );
		sprintf( pwm_ctl->misc_opt->logbuffer, "softpwminit returns %d", res);
		soft_pwm_log( pwm_ctl->misc_opt->logbuffer, 1 );


		res = soft_pwm_create_thread( pwm_ctl, SOFTPWM_R_THREAD );
		sprintf( pwm_ctl->misc_opt->logbuffer, "create thead R returns %d", res);
		soft_pwm_log( pwm_ctl->misc_opt->logbuffer, 1 );

		res = soft_pwm_create_thread( pwm_ctl, SOFTPWM_G_THREAD );
		sprintf( pwm_ctl->misc_opt->logbuffer, "create thead G returns %d", res);
		soft_pwm_log( pwm_ctl->misc_opt->logbuffer, 1 );

		res = soft_pwm_create_thread( pwm_ctl, SOFTPWM_B_THREAD );
		sprintf( pwm_ctl->misc_opt->logbuffer, "create thead B returns %d", res);
		soft_pwm_log( pwm_ctl->misc_opt->logbuffer, 1 );

		res = soft_pwm_create_thread( pwm_ctl, SOFTPWM_TCP_THREAD );
		sprintf( pwm_ctl->misc_opt->logbuffer, "create thead TCP returns %d", res);
		soft_pwm_log( pwm_ctl->misc_opt->logbuffer, 1 );


		/* Destroy the thread attributes object, since it is no
			longer needed */

		if( pthread_attr_destroy(&pwm_ctl->misc_opt->attr) != 0 )
		{
			soft_pwm_log( "", 1 );
		}

		soft_pwm_wait_threads( pwm_ctl );

		softpwm_terminate( pwm_ctl );

		soft_pwm_cancel_thread( pwm_ctl, SOFTPWM_R_THREAD );
		soft_pwm_cancel_thread( pwm_ctl, SOFTPWM_G_THREAD );
		soft_pwm_cancel_thread( pwm_ctl, SOFTPWM_B_THREAD );
		soft_pwm_cancel_thread( pwm_ctl, SOFTPWM_TCP_THREAD );

		clock_gettime( CLOCK_REALTIME, &pwm_ctl->misc_opt->end_time );
		free(pwm_ctl->misc_opt->tinfo);

		soft_pwm_release( pwm_ctl );
	}
	else
	{
perror("init failed");
	}

}



