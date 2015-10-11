
#ifndef RGBSOFTPWM_H
#define RGBSOFTPWM_H

#ifdef __cplusplus
extern "C" {
#endif

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


#define SOFTPWM_UNITTYPE_NONE		1
#define SOFTPWM_UNITTYPE_HZ		2
#define SOFTPWM_UNITTYPE_DGR_C		3
#define SOFTPWM_UNITTYPE_DGR_F		4
#define SOFTPWM_UNITTYPE_PERCENT	5
#define SOFTPWM_UNITTYPE_VOLT		6
#define SOFTPWM_UNITTYPE_AMPERE		7
#define SOFTPWM_UNITTYPE_BAR		8

#define SOFTPWM_UNITPRE_NONE	1
#define SOFTPWM_UNITPRE_KILO	2
#define SOFTPWM_UNITPRE_MEGA	3
#define SOFTPWM_UNITPRE_GIGA	4
#define SOFTPWM_UNITPRE_MILLI	5
#define SOFTPWM_UNITPRE_MICRO	6
#define SOFTPWM_UNITPRE_NANO	7
#define SOFTPWM_UNITPRE_PICO	8

#define SOFTPWM_VALTYPE_UCHAR	 1
#define SOFTPWM_VALTYPE_CHAR	 2
#define SOFTPWM_VALTYPE_USHORT	 3
#define SOFTPWM_VALTYPE_SHORT	 4
#define SOFTPWM_VALTYPE_UINT	 5
#define SOFTPWM_VALTYPE_INT	 6
#define SOFTPWM_VALTYPE_ULONG	 7
#define SOFTPWM_VALTYPE_LONG	 8
#define SOFTPWM_VALTYPE_FLOAT	 9
#define SOFTPWM_VALTYPE_DOUBLE	10


#define LOG_CRITICAL_K	 1
#define LOG_ERROR_K	 2
#define LOG_WARN_K	 3
#define LOG_MSG_K	 4
#define LOG_DUMP_K	 5
#define LOG_CRITICAL	 6
#define LOG_ERROR	 7
#define LOG_WARN	 8
#define LOG_MSG		 9
#define LOG_DUMP	10


#define SOFTPWM_THREAD_INIT	 1
#define SOFTPWM_THREAD_RUNNING	 2
#define SOFTPWM_THREAD_FINISHED	 0
#define SOFTPWM_THREAD_FAILED	-1

#define SOFTPWM_R_THREAD	0
#define SOFTPWM_G_THREAD	1
#define SOFTPWM_B_THREAD	2
#define SOFTPWM_TCP_THREAD	3
#define SOFTPWM_NUM_THREADS	4
#define DEFAULT_STACKSIZE	0x100000

#define DEFAULT_INTENSITY_R	128
#define DEFAULT_INTENSITY_G	128
#define DEFAULT_INTENSITY_B	128

#define DEFAULT_FREQUENCY_R	20
#define DEFAULT_FREQUENCY_G	20
#define DEFAULT_FREQUENCY_B	20

#define DEFAULT_GPIONAME_R	"23"
#define DEFAULT_GPIONAME_G	"24"
#define DEFAULT_GPIONAME_B	"25"

#define DEFAULT_PWM_LISTENPORT	3434
#define DEFAULT_PWM_FREQUENCY	20
#define DEFAULT_PWM_INCVAL	10
#define DEFAULT_PWM_DECVAL	10
#define DEFAULT_PWM_BOARDREV	2
#define DEFAULT_PWM_LOGFACILITY	LOG_LOCAL0;
#define DEFAULT_PWM_LOGOPTIONS	 (LOG_PID | LOG_CONS| LOG_NDELAY)
#define DEFAULT_PWM_DAEMONNAME	"softpwm_d"
#define DEFAULT_PWM_VLEVEL	4
#define DEFAULT_PWM_PIDFILENAME	"/var/log/softpwm.lck"
#define DEFAULT_PWM_CFGFILENAME	"~/.softpwm.cfg"

#define SOFTPWM_HZ_MULTI	      1
#define SOFTPWM_KHZ_MULTI	   1000
#define SOFTPWM_MHZ_MULTI	1000000

#define SOFTPWM_TCP_CMD_NOT_FOUND	 0
#define SOFTPWM_TCP_CMD_CLOSE		 1
#define SOFTPWM_TCP_CMD_EXIT		 2
#define SOFTPWM_TCP_CMD_HELP		 3
#define SOFTPWM_TCP_CMD_SET_RINTENSITY	 4
#define SOFTPWM_TCP_CMD_SET_GINTENSITY	 5
#define SOFTPWM_TCP_CMD_SET_BINTENSITY	 6
#define SOFTPWM_TCP_CMD_GET_RINTENSITY	 8
#define SOFTPWM_TCP_CMD_GET_GINTENSITY	 9
#define SOFTPWM_TCP_CMD_GET_BINTENSITY	10
#define SOFTPWM_TCP_CMD_GET_ALL		12
#define SOFTPWM_TCP_CMD_INCR_RINTENSITY	13
#define SOFTPWM_TCP_CMD_INCR_GINTENSITY	14
#define SOFTPWM_TCP_CMD_INCR_BINTENSITY	15
#define SOFTPWM_TCP_CMD_DECR_RINTENSITY	17
#define SOFTPWM_TCP_CMD_DECR_GINTENSITY	18
#define SOFTPWM_TCP_CMD_DECR_BINTENSITY	19
#define SOFTPWM_TCP_CMD_GET_INCR	21
#define SOFTPWM_TCP_CMD_SET_INCR	22
#define SOFTPWM_TCP_CMD_GET_DECR	23
#define SOFTPWM_TCP_CMD_SET_DECR	24
#define SOFTPWM_TCP_CMD_INCR_RFREQ	25
#define SOFTPWM_TCP_CMD_INCR_GFREQ	26
#define SOFTPWM_TCP_CMD_INCR_BFREQ	27
#define SOFTPWM_TCP_CMD_DECR_RFREQ	28
#define SOFTPWM_TCP_CMD_DECR_GFREQ	29
#define SOFTPWM_TCP_CMD_DECR_BFREQ	30
#define SOFTPWM_TCP_CMD_SET_RFREQ	31
#define SOFTPWM_TCP_CMD_SET_GFREQ	32
#define SOFTPWM_TCP_CMD_SET_BFREQ	33
#define SOFTPWM_TCP_CMD_GET_RFREQ	34
#define SOFTPWM_TCP_CMD_GET_GFREQ	35
#define SOFTPWM_TCP_CMD_GET_BFREQ	36


struct thread_info {			/* Used as argument to thread_start() */
	pthread_t	thread_id;	/* ID returned by pthread_create() */
	int		thread_num;	/* Application-defined thread # */
	char*		argv_string;	/* From command-line argument */
};

typedef struct cmd_list_t {
unsigned char* shortcut;
unsigned char* command;
unsigned char* description;
short cmd_number;
} cmd_list;

typedef struct pwm_thread_data_t {
	int intensity;
	int frequency;
	int f_multi;
	uint8_t pin;
} pwm_thread_data;

typedef struct pwm_server_data_t {
	int rctl_listener;
	int rctl_cmd_interface;
	struct sockaddr_in rctl_serveraddr;
	struct sockaddr_in rctl_remoteaddr;
	struct timeval rctl_timeout;
	unsigned char *p_rctl_recv_buf;
	unsigned char rctl_recv_buf[512];
	unsigned char *p_rctl_resp_buf;
	unsigned char rctl_resp_buf[512];
	int rctl_errno;
	struct cmd_list_t *cmd;
	unsigned char *p_rctl_logbuffer;
	unsigned char rctl_logbuffer[8192];
} pwm_server_data;

typedef struct pwm_general_data_t {
	struct thread_info *tinfo;
	pthread_attr_t attr;
	int stack_size;
	int pwm_r_ctl_status;
	int pwm_g_ctl_status;
	int pwm_b_ctl_status;
	int pwm_tcp_ctl_status;
	int inc_val;
	int dec_val;
	short board_revision;
	int log_facility;
	int log_options;
	unsigned char *daemon_name;
	pid_t daemon_pid;
	struct timespec call_time;
	struct timespec start_time;
	struct timespec end_time;
	int exit_code;
	int last_error;
	short verbose_level;
	unsigned char *pid_file_name;
	unsigned char *p_log;
	unsigned char logbuffer[8192];
	unsigned char *p_cfg;
	unsigned char config_file_name[PATH_MAX];
} pwm_general_data_t;

typedef struct soft_pwm_options_t {
	struct pwm_thread_data_t *r_channel;
	struct pwm_thread_data_t *g_channel;
	struct pwm_thread_data_t *b_channel;
	struct pwm_server_data_t *rctl;
	struct pwm_general_data_t *misc_opt;
} soft_pwm_options;

typedef struct phys_bcm_t {
short phys_pin;
short bcm_pin;
} phys_bcm;

typedef union c_val_t {
unsigned char	uc;
char 		c;
unsigned short	us;
short		s;
unsigned int	ui;
int		i;
unsigned long	ul;
long		l;
float		f;
double		d;
} c_val;

typedef struct num_val_t {
short	val_type;
c_val	val;
short	unit_type;
short	unit_pre;
} num_val;




extern struct cmd_list_t cmd[];
extern struct soft_pwm_options_t *global_pwm_ctl;

extern void set_pwm_default_options( struct soft_pwm_options_t *pwm_ctl );
extern void clear_pwm_options( struct soft_pwm_options_t *pwm_ctl );
extern void get_arguments ( int argc, char **argv, struct soft_pwm_options_t *pwm_ctl );
extern void read_config(void);
extern struct soft_pwm_options_t *soft_pwm_release( struct soft_pwm_options_t *pwm_ctl );
extern void soft_pwm_log(unsigned char *outbuf, int logflag);
extern void soft_pwm_showargs( struct soft_pwm_options_t *pwm_ctl );
extern struct soft_pwm_options_t *soft_pwm_init(void);
extern int pin2bcm( int pin );
extern int soft_pwm_create_thread( struct soft_pwm_options_t *pwm_ctl, int ThreadId );
extern void soft_pwm_wait_threads( struct soft_pwm_options_t *pwm_ctl );
extern int soft_pwm_cancel_thread( struct soft_pwm_options_t *pwm_ctl, int thread_num );
static void catch_SIGUSR1(int val);
extern int soft_pwm_daemonize( struct soft_pwm_options_t *pwm_ctl );
static void *run_r_channel_thread();
static void *exit_r_channel_channel_thread( int exit_code );
static void *run_g_channel_thread();
static void *exit_g_channel_channel_thread( int exit_code );
static void *run_b_channel_thread();
static void *exit_b_channel_channel_thread( int exit_code );
static void *run_rgb_channel_thread(int threadId, struct soft_pwm_options_t *pwm_ctl );

static void *exit_rgb_channel_thread(int threadId, int exit_code, struct soft_pwm_options_t *pwm_ctl );


static void *exit_tcp_thread( int exit_code );
static void *run_tcp_thread();
static int prepare_server_socket( struct soft_pwm_options_t *pwm_ctl );
extern int do_tcp_command( struct soft_pwm_options_t *pwm_ctl );
extern int get_value( unsigned char *recv_buf, struct num_val_t *value );
extern int scan_tcp_command( struct soft_pwm_options_t *pwm_ctl );
extern void compact_input( unsigned char *inp_str );

extern int set_rchannel( struct num_val_t value, struct soft_pwm_options_t *pwm_ctl );
extern int set_gchannel( struct num_val_t value, struct soft_pwm_options_t *pwm_ctl );
extern int set_bchannel( struct num_val_t value, struct soft_pwm_options_t *pwm_ctl );
extern int set_frequency( struct num_val_t value, struct soft_pwm_options_t *pwm_ctl );
extern int set_incr( struct num_val_t value, struct soft_pwm_options_t *pwm_ctl );
extern int set_decr( struct num_val_t value, struct soft_pwm_options_t *pwm_ctl );
extern int incr_rchannel( struct soft_pwm_options_t *pwm_ctl );
extern int incr_gchannel( struct soft_pwm_options_t *pwm_ctl );
extern int incr_bchannel( struct soft_pwm_options_t *pwm_ctl );
extern int incr_frequency( struct soft_pwm_options_t *pwm_ctl );
extern int decr_rchannel( struct soft_pwm_options_t *pwm_ctl );
extern int decr_gchannel( struct soft_pwm_options_t *pwm_ctl );
extern int decr_bchannel( struct soft_pwm_options_t *pwm_ctl );
extern int decr_frequency( struct soft_pwm_options_t *pwm_ctl );

extern int softpwm_initialize( struct soft_pwm_options_t *pwm_ctl );
extern int setup_r_pin( struct soft_pwm_options_t *pwm_ctl );
extern int setup_g_pin( struct soft_pwm_options_t *pwm_ctl );
extern int setup_b_pin( struct soft_pwm_options_t *pwm_ctl );
extern int reinit_r_pin( struct soft_pwm_options_t *pwm_ctl );
extern int reinit_g_pin( struct soft_pwm_options_t *pwm_ctl );
extern int reinit_b_pin( struct soft_pwm_options_t *pwm_ctl );


#ifdef __cplusplus
}
#endif

#endif /* RGBSOFTPWM_H */

