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

#include "cc1111.h"
#include "main.h"
#include "usb.h"
#include "intel_hex.h"

void clock_init()
{
	// Switch system clock to crystal oscilator
	CLKCON = (CLKCON & ~CLKCON_OSC_MASK) | (CLKCON_OSC_XTAL);

	while (!(SLEEP & SLEEP_XOSC_STB)) {}

	// Crank up the timer tick and system clock speed
	CLKCON = ((CLKCON & ~(CLKCON_TICKSPD_MASK | CLKCON_CLKSPD_MASK)) |
		  (CLKCON_TICKSPD_1 | CLKCON_CLKSPD_1));

	while ((CLKCON & (CLKCON_TICKSPD_MASK|CLKCON_CLKSPD_MASK)) !=
	       (CLKCON_TICKSPD_1 | CLKCON_CLKSPD_1)
	      ) {}
}

#ifdef STDIO
void putchar(char c) {
    usb_putchar(c);
}
#endif
 
void delay (unsigned char n) {
	unsigned char i = 0;
	unsigned char j = 0;
 
	n <<= 1;
	while (--n != 0)
		while (--i != 0)
		  while (--j != 0)
			  nop();
}
 
void main ()
{
  char c;
  
  clock_init();
  
  // Setup LED and turn it off
  P1DIR |= 2;
  P1_1 = 0;
  
  usb_init();
  
  // Enable interrupts
	EA = 1;
  
  // Bring up the USB link
	P1DIR |= 1;
	P1 |= 1;
  
  while (1) 
  {
    c = usb_getchar();
    usb_putchar(c);
    usb_flush();
    //delay(2);
	}
}
