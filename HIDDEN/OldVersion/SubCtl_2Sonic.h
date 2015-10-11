/* *******************************************************************
 *
 * SubCtl_2Sonic.h
 *
 * (C) 2014 Dreamshader (Dirk Schanz)
 *
 * control of an ultrasonic subcontroller (pro mini with two us-sensors)
 *     definitions and prototypes
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

#ifndef _SUBCTL2SONIC_H_
#define _SUBCTL2SONIC_H_

//
// constant definitions
//
#define CONDITION_2SONIC_R	3
#define CONDITION_2SONIC_Y	4
#define CONDITION_2SONIC_G	5
// defaults
// error codes

//
// structures/unions
//

//
// function prototypes
//
extern int Sub2SonicCritical;

#ifndef UBUNTU
void alert2Sonic(int gpio, int level, uint32_t tick ); //void* udata
int initialize_2sonic(  srv_opts_t *options );
#endif // NOT UBUNTU


#endif // _SUBCTL2SONIC_H_

#ifdef __cplusplus
}
#endif


