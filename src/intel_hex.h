/*
 * CC Bootloader - Intel HEX file format functions 
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

#ifndef _INTEL_HEX_H_
#define _INTEL_HEX_H_

#define IHX_OK              0
#define IHX_INVALID         1
#define IHX_BAD_CHECKSUM    2
#define IHX_BAD_ADDRESS     3
#define IHX_BAD_RECORD_TYPE 4
#define IHX_RECORD_TOO_LONG 5

#define IHX_MAX_LEN 0x10

#define HEX_INVALID 0xFF

#define IHX_RECORD_DATA 0x00
#define IHX_RECORD_EOF  0x01

// Custom record types used to implement some extra bootloader functionality.

// Reset record will reset the page erase map which usually ensures each page is only
// erased once, allowing for random writes but preventing overwriting of data already written
// this session.
// :00000022DE
#define IHX_RECORD_RESET  0x22

// Erases all of the user code flash pages
// :00000023DD
#define IHX_RECORD_ERASE_ALL  0x23

// Erases a single page of the user code flash
// :01000024xxyy
// xx - Page number, yy - Checksum
#define IHX_RECORD_ERASE_PAGE  0x24

// Reads back a section of flash in Intel HEX format
// :04xxxx24yyyyzz
// xxxx - Start address, yyyy - Num bytes to read, zz - Checksum
#define IHX_RECORD_READ  0x25


uint8_t hex4(char c);
uint8_t hex8(char s[]);
uint16_t hex16(char s[]);

uint8_t ihx_check_line(char line[]);
void ihx_readline(char line[]);
void ihx_write(char line[]);
uint8_t ihx_record_type(char line[]);
uint8_t ihx_data_byte(char line[], uint8_t n);

#endif // _INTEL_HEX_H_