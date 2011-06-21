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

uint8_t bootloader_running = 1;

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

void jump_to_user() {
  // Disable all interrupts
  EA = 0;
  IEN0 = IEN1 = IEN2 = 0;
  // Flag bootloader not running
  bootloader_running = 0;
  // Jump to user code
  __asm
    ljmp #USER_CODE_BASE
  __endasm;
}

void bootloader_main ()
{
  __xdata char buff[100];
  uint8_t ihx_status;
  
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
    ihx_readline(buff);
    
    ihx_status = ihx_check_line(buff);
    
    if (ihx_status == IHX_OK) {
      switch (ihx_record_type(buff)) {
        case IHX_RECORD_DATA:
          ihx_write(buff);
          break;
        case IHX_RECORD_EOF:
          usb_putstr("Jumping to user code\n");
          jump_to_user();
          break;
        default:
          // Return the error code for unknown type in this case too
          usb_putchar(IHX_BAD_RECORD_TYPE + '0');
          usb_flush();
          break;
      }
    }
    usb_putchar(ihx_status + '0');
    usb_flush();
	}
}
