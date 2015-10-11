/* *******************************************************************
 *
 * TelnetThread.h
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

#ifndef _TELNETTHREAD_H_
#define _TELNETTHREAD_H_

void *run_telnet_thread(void *runOptions);

#endif // _TELNETTHREAD_H_

#ifdef __cplusplus
}
#endif





