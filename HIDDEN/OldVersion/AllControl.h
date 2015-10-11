/* *******************************************************************
 *
 * AllControl.h
 *
 * (C) 2014 Dreamshader (Dirk Schanz)
 *
 * part of a multi-process server.
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
#define PID_FILE	"/var/lock/AllControl.pid"
#define INIT_FD			  -1
#define DEFAULT_DAEMON		   1
#define DEFAULT_RUNMODE		RUN_FOREGROUND
#define DEFAULT_DAEMONNAME	"AllControl"

// rs232 related
#define INIT_RS232_FD		  -1
#define DEFAULT_RS232_BAUD	9600
#define DEFAULT_RS232_PARITY	  'N'
#define DEFAULT_RS232_DATAB	   8
#define DEFAULT_RS232_STOPB	   1
#define DEFAULT_RS232_HANDSHAKE	  'N'

// error codes
#define E_NOERR		 0
#define E_FAIL		-1
#define E_F_GETFL	-2
#define E_F_SETFL	-3
#define E_FORK		-4
#define E_SETSID	-5
#define E_NULLPTR	-6
#define E_NOMEM		-7


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


#define REQ_QUIT	1
#define REQ_EXIT	2
#define REQ_DEBUG	3
#define REQ_VERBOSE	4
#define REQ_DEVICE	5

// subcontroller section
//
// constant definitions
//

#define I2C_BUS_NO		   1	// i2c bus ...
#define I2C_MSG_LEN		  32	// max len of an i2c message

#define I2C_SLAVE_MOTOR		0x09	// slave addr of subcontroller
#define PIN_ALARM_MOTOR		  15	// physical pin #15

#define I2C_SLAVE_TFT		0x0a	// slave addr of subcontroller
#define PIN_ALARM_TFT		  16	// physical pin #16

#define I2C_SLAVE_2SONIC	0x0b	// slave addr of subcontroller
#define PIN_ALARM_2SONIC	  11	// physical pin #11

#define I2C_SLAVE_4SONIC	0x0c	// slave addr of subcontroller
#define PIN_ALARM_4SONIC	  18	// physical pin #18

#define I2C_SLAVE_GYRO		0x0d	// slave addr of subcontroller
#define PIN_ALARM_GYRO		  22	// physical pin #22

// completely free usable:
// Pin # 16, 18, 22 -> GPIO 23, 24, 25
// Pin # 11, 15 -> GPIO 17, 22
// reserved pins:
// Pin #  1, 17 -> 3V3
// Pin #  2,  4 -> 5V
// Pin #  6,  9, 14, 20, 25 -> GND
// alternate function pins
// Pin #  3,  5 -> SDA, SCL (GPIO  2,  3)
// Pin #  8, 10 -> TxD, RxD (GPIO 14, 15)
// Pin #  7, 12, 13 -> GPCLK0, PCM_CLK, PCM_DOUT (GPIO 4, 18, 27)
// Pin # 19, 21, 23 -> MOSI, MISO, SCLK (GPIO 10, 9, 11)
// Pin # 24, 26 ->  CE0, CE1 (GPIO 8, 7)

#define SUBCTL_2SONIC
// #undef SUBCTL_2SONIC
// #define SUBCTL_4SONIC
#undef SUBCTL_4SONIC
// #define SUBCTL_MOTOR
#undef SUBCTL_MOTOR
// #define SUBCTL_GYRO
#undef SUBCTL_GYRO
// #define SUBCTL_TFT
#undef SUBCTL_TFT

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

typedef struct _known_cmd {
char *cmd;
char *fmt;
short cmdCode;
} known_cmd_t;

typedef struct clienttab {
char *device;
char *mode;
pid_t pid;
} clienttab_t;

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
#ifndef UBUNTU
// pigpio stuff
int pigpioInitialized;
// iic-stuff
int i2cHandle2Sonic;
int i2cHandle4Sonic;
int i2cHandleTft;
int i2cHandleMotor;
int i2cHandleGyro;
char i2cInBuf[I2C_MSG_LEN];
char i2cOutBuf[I2C_MSG_LEN];
#endif // NOT UBUNTU
// connection stuff - one rs232 only, yet
serial_param serial_dev;
// child list - parent process only
int child_entry;
clienttab_t **childs;
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


