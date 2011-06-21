/*
 * CC Bootloader - Main 
 *
 * Fergus Noble (c) 2011
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */
 
#ifndef _MAIN_H_
#define _MAIN_H_

// The address of the start of the user code section
// This must be a multiple of 1kb to fit on a flash page boundary
// !!! NOTE: at the moment you must also change this in start.asm IVT !!!
#define USER_CODE_BASE (4*1024)
#define USER_FIRST_PAGE (USER_CODE_BASE/1024)

// Change to match the CC1111 part you are using
#define FLASH_SIZE 0x8000
//(32*1024)
#define FLASH_PAGES (FLASH_SIZE/1024)

// Useful for printf etc. but uses a bunch of code space
//#define STDIO
#ifdef STDIO
#include <stdio.h>
#else
 // TODO: put prototypes here maybe so things don't break when 
 // this is disabled?
#endif

#define nop()	__asm nop __endasm;

extern uint8_t bootloader_running;

#endif // _MAIN_H_