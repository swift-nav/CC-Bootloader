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
#include "hal.h"
#include "flash.h"
#include "intel_hex.h"

uint8_t bootloader_running = 1;

void clock_init()
{
	// Switch system clock to crystal oscilator
	CLKCON = (CLKCON & ~CLKCON_OSC_MASK) | (CLKCON_OSC_XTAL);

	while (!(SLEEP & SLEEP_XOSC_STB)) {}

	// Crank up the timer tick and system clock speed
	CLKCON = ((CLKCON & ~(CLKCON_TICKSPD_MASK | CLKCON_CLKSPD_MASK)) |
		  (CLKCON_TICKSPD_1_128 | CLKCON_CLKSPD_1));

	while ((CLKCON & (CLKCON_TICKSPD_MASK|CLKCON_CLKSPD_MASK)) !=
	       (CLKCON_TICKSPD_1_128 | CLKCON_CLKSPD_1)
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

uint8_t check_for_payload() {
  if (*((__xdata uint8_t*)USER_CODE_BASE) == 0xFF)
    return 0;
  else
    return 1;
}

void jump_to_user() {
  // Disable all interrupts
  EA = 0;
  IEN0 = IEN1 = IEN2 = 0;
  
  
  if (check_for_payload()) {
    // Bring down the USB link
    usb_down();
  
    // Flag bootloader not running
    bootloader_running = 0;
    // Jump to user code
    __asm
      ljmp #USER_CODE_BASE
    __endasm;
    while (1) {}
  } else {
    #ifdef RFCAT
    return; // need to run bootloader!
    #else
    // Oops, no payload. We're stuck now!
    led_on();
    while (1) {}
    #endif
  }
}

#ifdef TIMER
void setup_timer1() {
  // Clear Timer 1 channel 1 and overflow interrupt flag
  // CPU interrupt flag (IRCON) for Timer 1 is cleared automatically by hardware.
  T1CTL &= ~T1CTL_OVFIF;
  
  // Enable Timer 1 interrupts by setting [IEN1.T1IE=1]
  IEN1 |= IEN1_T1IE;
  
  T1CC0H = TIMER_TIMEOUT;
  T1CC0L = 0x00;
  
  // set Timer 1 to modulo mode
  T1CTL = TxCTL_OVFIM | TxCTL_DIV_128 | TxCTL_MODE_MODULO;
}
#endif

#ifdef TIMER
void disable_timer1() {
  // Disable Timer 1 interrupt
  IEN1 &= ~IEN1_T1IE;
}
#endif

#ifdef TIMER
void timer1_isr() __naked {
  T1CTL &= ~T1CTL_OVFIF;
  
  // We need to issue a RETI instruction to
  // get out of interrupt mode. We push the
  // address of a dummy label onto the stack
  // to spoof the return address of the RETI
  // instruction.
  __asm
    ; Push address of label just past RETI 
    mov a, #<silly_reti ; Low byte first
    push acc
    mov a, #>silly_reti ; High byte
    push acc
    ; Issue RETI to get out of interrupt
    ; Should return to silly_reti
    reti
  silly_reti:
  __endasm;
  
  // Now go to user code, don't worry about the fact we have
  // frigged the stack, the user code will just reset it anyway.
  jump_to_user();
}
#endif

void timer1_isr_forward() __naked {
  #ifdef TIMER
  __asm
  	push acc
  	mov	a, _bootloader_running
  	jnz	timer1_isr_forward_bootloader
  	; Bootloader not running, jump into the payload ISR
  	pop acc
  	ljmp #(USER_CODE_BASE+0x4B)
  timer1_isr_forward_bootloader:
  	pop acc
  	ljmp	_timer1_isr
  __endasm;
  #else
  __asm
  	ljmp #(USER_CODE_BASE+0x4B)
  __endasm;  
  #endif
}

uint8_t want_bootloader() {
  // Check if we want to the bootloader to run
  // Here is the place to check for things like USB power and jump straight to
  // user code under some conditions.
  
  /*
  // Check for USB +5V, if not present go straight to user code
  PxDIR &= ~y;
  if (!Px_y)
    return 0;
  */

  #ifdef RFCAT 
  // we use the unused I2S SFRs as semaphores.
  // this would be safe even if I2S is in use as they should be reconfigured by 
  // user code 
  if(I2SCLKF2 == 0x69) 
    return 1;
  // no thanks
  return 0;
  #else
  return 1;
  #endif
}

void bootloader_main ()
{
  __xdata char buff[100];
  uint8_t ihx_status;
  uint16_t read_start_addr, read_len;

#ifdef RFCAT
  // use I2S SFR to signal that bootloader is present
  I2SCLKF0= 0xF0;
  I2SCLKF1= 0x0D;

  setup_button();

  #ifdef RFCAT_DONSDONGLE
  if (CC1111EM_BUTTON != BUTTON_PRESSED && !want_bootloader())
  #endif

  #ifdef RFCAT_CHRONOS
  if (CC1111CHRONOS_PIN_DC != GROUNDED && !want_bootloader())
  #endif

  #ifdef RFCAT_YARDSTICKONE
  if (CC1111YSONE_PIN_DC != GROUNDED && !want_bootloader())
  #endif

#else
  if (!want_bootloader())
#endif
    jump_to_user();
#ifdef RFCAT
  // reset semaphore 
  I2SCLKF2= 0x00;
#endif

  clock_init();
  
  setup_led();
  
  // Setup timer if enabled
  #ifdef TIMER
  setup_timer1();
  #endif
  
  usb_init();
  
  // Enable interrupts
  EA = 1;
  
  // Bring up the USB link
  usb_up();
  led_on();
 
  while (1) 
  {
    ihx_readline(buff);
    
    // Got something over USB, disable the timer
    #ifdef TIMER
    disable_timer1();
    #endif
    
    ihx_status = ihx_check_line(buff);
    
    if (ihx_status == IHX_OK) {
      switch (ihx_record_type(buff)) {
        case IHX_RECORD_DATA:
          ihx_write(buff);
          usb_putchar('0');
          usb_flush();
          break;
        case IHX_RECORD_EOF:
          jump_to_user();
          break;
        case IHX_RECORD_RESET:
          // Reset record will reset the page erase map which usually ensures each page is only
          // erased once, allowing for random writes but preventing overwriting of data already written
          // this session.
          flash_reset();
          usb_putchar('0');
          usb_flush();
          break;
        case IHX_RECORD_ERASE_ALL:
          // Erase all user flash pages
          flash_erase_all_user();
          usb_putchar('0');
          usb_flush();
          break;
        case IHX_RECORD_ERASE_PAGE:
          // Erase flash page
          flash_erase_page(ihx_data_byte(buff, 0));
          usb_putchar('0');
          usb_flush();
          break;
        case IHX_RECORD_READ:
          // Read out a section of flash over USB
          read_start_addr = ihx_record_address(buff);
          read_len = (ihx_data_byte(buff, 0)<<8) + ihx_data_byte(buff, 1);
          usb_putchar('\n');
          ihx_read_print((__xdata uint8_t*)read_start_addr, read_len);
          break;
        default:
          // Return the error code for unknown type in this case too
          usb_putchar(IHX_BAD_RECORD_TYPE + '0');
          usb_flush();
          break;
      }
    } else {
      usb_putchar(ihx_status + '0');
      usb_flush();
      }
  }
}
