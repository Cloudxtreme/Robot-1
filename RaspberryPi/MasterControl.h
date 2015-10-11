/* *******************************************************************
 *
 * MasterControl.h
 *
 * (C) 2014 Dreamshader (Dirk Schanz)
 *
 * part of the master control process for a robot
 *     definitions for main structures
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

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _ALLCONTROL_H_
#define _ALLCONTROL_H_

#define RUN_STOP		   0
#define RUN_DAEMON		   2
#define RUN_FOREGROUND		   3

// defaults
#define DEFAULT_VERBOSE 	  10
#define DEFAULT_DEBUG 		  10
#define DEFAULT_PORT 		6633
#define DEFAULT_BACKLOG		   5
#define PID_FILE	"/var/lock/MasterControl.pid"
#define INIT_FD			  -1
#define DEFAULT_DAEMON		   1
#define DEFAULT_RUNMODE		RUN_FOREGROUND
#define DEFAULT_DAEMONNAME	"MasterControl"

// thread related
#define CTRL_THREAD_INIT	 1
#define CTRL_THREAD_RUNNING	 2
#define CTRL_THREAD_FINISHED	 0
#define CTRL_THREAD_FAILED	-1
#define MASTERCTL_NUM_THREADS	 3

#define MAINCTL_THREAD_NUM	0
#define API_THREAD_NUM		1
#define TELNET_THREAD_NUM	2

#define DEFAULT_STACKSIZE	0x100000

// error codes
#define E_NOERR		 0
#define E_FAIL		-1
#define E_F_GETFL	-2
#define E_F_SETFL	-3
#define E_FORK		-4
#define E_SETSID	-5
#define E_NULLPTR	-6
#define E_NOMEM		-7
#define E_THREADINVAL   -8
#define E_NOCOMMAND	-9

// logging stuff - levels
#define LOG_FATL	-5
#define LOG_CRITERROR	-4
#define LOG_ERROR	-3
#define LOG_CRITWARN	-2
#define LOG_WARN	-1
#define LOG_INF_0	 1
#define LOG_INF_1	 2
#define LOG_INF_2	 3
#define LOG_DBG		 4

#define CTLREQ_EXIT	1
#define CTLREQ_API	2
#define CTLREQ_UNKNOWN	3

//
// end of constant section
//

typedef struct copyright {
char *module;
char *version;
char *author;
char *aka;
char *license;
} copyright_t;

typedef struct _master_cmd {
char *cmd;
short cmdCode;
} master_cmd_t;

struct thread_info {		/* argument to thread_start() */
pthread_t	thread_id;	/* ID returned by pthread_create() */
int		thread_num;	/* Application-defined thread # */
void*		options;	/* main control options */
};

typedef struct srv_opts {
char *daemon_name;
// run mode ... 
int run_mode;
// return and exit-codes
int error_code;
int sys_errno;
// options given by caller or use defaults
short verbose_level;
short debug_level;
short listen_port;
short backlog;
short run_as_daemon;
// run information for logging etc.
struct timespec start_time;
struct timespec end_time;
// log flags for using syslog
int log_facility;
int log_options;
// name of file to hold pid
char *pid_file_name;
// logging stuff 
char *p_log;
char logbuffer[8192];
// socket stuff
int socket;
int cli_socket;
socklen_t sock_len;
struct sockaddr_in local_addr;
struct sockaddr_in remote_addr;
int rcv_len;
char *p_rcv_buf;
char rcv_buf[512];
int rsp_len;
char *p_rsp_buf;
char rctl_rsp_buf[512];
// thread related
int MainThreadStatus;
int ApiThreadStatus;
int TelnetThreadStatus;
volatile int mainctl_run;
volatile int api_run;
volatile int telnet_run;
struct thread_info *tinfo;
pthread_attr_t tattr;
int stack_size;
} srv_opts_t;

// function prototypes
//
void closelogfile( srv_opts_t *options );
void do_log( srv_opts_t *options, int level );
void ErrHndl( srv_opts_t *options, int errCode );
void cleanup_and_exit( srv_opts_t *options );
int nonblock(int fd);
int get_request( srv_opts_t *options );
int send_response( srv_opts_t *options );

#endif // _ALLCONTROL_H_

#ifdef __cplusplus
}
#endif


