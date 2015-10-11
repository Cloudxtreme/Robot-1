/* *******************************************************************
 *
 * CommRs232.h
 *
 * (C) 2014 Dreamshader (Dirk Schanz)
 *
 *  part of a multi-process server.
 *     defines for handling a rs232 connection.
 *     
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

#ifndef _COMMRS232_H_
#define _COMMRS232_H_

#ifdef NEVERDEF
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/signal.h>
#include <sys/param.h>
#include <fcntl.h>
#include <termios.h>
#endif // NEVERDEF

typedef struct serial_param_t {
int dev_fd;
char *device_name;
struct termios oldsettings;
struct termios checksettings;
struct termios newsettings;
int baud;
char databit;
char parity;
char stoppbits;
char handshake;
char async;
char *dev_mode;
char logbuffer[128];
} serial_param;

int rs232Open( struct serial_param_t *ctl_param );
void rs232Close( struct serial_param_t *ctl_param );
int rs232Reset( struct serial_param_t *ctl_param );
int rs232GetParam( struct serial_param_t *ctl_param, struct termios *mytio );
int rs232SetUpRaw( struct serial_param_t *ctl_param, struct termios *mytio );
int rs232SetUpParam( struct serial_param_t *ctl_param, struct termios *mytio );
int rs232SetUpBaudrate( struct serial_param_t *ctl_param,
                        struct termios *mytio );
int rs232SetUpDatabits( struct serial_param_t *ctl_param,
                        struct termios *mytio );
int rs232SetUpParity( struct serial_param_t *ctl_param,
                      struct termios *mytio );
int rs232SetUpStoppbits( struct serial_param_t *ctl_param,
                         struct termios *mytio );
int rs232SetUpHandshake( struct serial_param_t *ctl_param,
                         struct termios *mytio );
int rs232CheckParam( struct serial_param_t *ctl_param, 
                struct termios *newtio, struct termios *savtio, int *ok );
int rs232SetUp( struct serial_param_t *ctl_param, 
                struct termios *newtio, struct termios *savtio, int *retries );


#define E_RS232_FAIL	-1

#endif // _COMMRS232_H_

#ifdef __cplusplus
}
#endif





