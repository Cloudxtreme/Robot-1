/* *******************************************************************
 *
 * MainCtlThread.h
 *
 * (C) 2014 Dreamshader (Dirk Schanz)
 *
 *  part of a multi-process server.
 *     definitions for child process
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


#ifndef _MAINCTLTHREAD_H_
#define _MAINCTLTHREAD_H_

//
// subcontroller section
//

//
// configuration definitions
//
#define SUBCTL_2SONIC
// #undef SUBCTL_2SONIC
// #define SUBCTL_4SONIC
#undef SUBCTL_4SONIC
#define SUBCTL_2MOTOR
// #undef SUBCTL_2MOTOR
// #define SUBCTL_4MOTOR
#undef SUBCTL_4MOTOR
// #define SUBCTL_GYRO
#undef SUBCTL_GYRO
// #define SUBCTL_TFT
#undef SUBCTL_TFT

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

// 
// thread specific
//


typedef struct _iic_vars {
// pigpio stuff
int pigpioInitialized;
// iic-stuff
int i2cHandle2Sonic;
void* pParam2sonic;
int i2cHandle4Sonic;
int i2cHandleTft;
int i2cHandle2Motor;
void* pParam2motor;
int i2cHandleGyro;
char i2cInBuf[I2C_MSG_LEN];
char i2cOutBuf[I2C_MSG_LEN];
} iic_vars_t, *p_iic_vars_t;

//
// prototypes
//
void *run_main_thread(void *runOptions);

#endif // _MAINCTLTHREAD_H_

#ifdef __cplusplus
}
#endif





