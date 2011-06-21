/*
 * CC Bootloader - Example Payload 
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

#include "../src/cc1111.h"

#define nop()	__asm nop __endasm;

void delay (unsigned char n) {
	unsigned char i = 0;
	unsigned char j = 0;
 
	n <<= 1;
	while (--n != 0)
		while (--i != 0)
		  while (--j != 0)
			  nop();
}
 
void main()
{  
  // Setup LED and turn it off
  P1DIR |= 2;
  P1_1 = 0;
  
  while (1) 
  {
    P1_1 ^= 1;
    delay(3);
	}
}
