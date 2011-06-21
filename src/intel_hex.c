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

#include "cc1111.h"
#include "intel_hex.h"
#include "usb.h"
#include "main.h"
#include "flash.h"

uint8_t hex4(char c) {
  // Converts a character representation of a hexadecimal nibble
  // into a uint8. If the nibble is invalid it will return 255.
  if (c >= '0' && c <= '9')
    return (uint8_t)(c - '0');
  else if (c >= 'A' && c <= 'F')
    return 10 + (uint8_t)(c - 'A');
  else if (c >= 'a' && c <= 'f')
    return 10 + (uint8_t)(c - 'a');
  else
    return HEX_INVALID;
}

uint8_t hex8(char s[]) {
  // Converts a string representation of a hexadecimal byte into a uint8.
  // TODO: handle the case of hex4 failing.
  return hex4(s[1]) + 16*hex4(s[0]);
}

uint16_t hex16(char s[]) {
  // Converts a string representation of a 16-bit hexadecimal word into a uint16.
  // TODO: handle the case of hex8 failing.
  return hex8(&s[2]) + 256*(uint16_t)hex8(&s[0]);
}

uint8_t ihx_check_line(char line[]) {
  // :ccaaaattxxxxss
  uint8_t byte_count, record_type, checksum, sum, i;
  uint16_t address;
  
  if (line[0] != ':')
    return IHX_INVALID;
  
  byte_count = hex8(&line[1]);
  address = hex16(&line[3]);
  record_type = hex8(&line[7]);

  if (byte_count > IHX_MAX_LEN)
    return IHX_RECORD_TOO_LONG;
  
  checksum = ihx_data_byte(line, byte_count);

  if (record_type > 0x01 && (record_type < 0x22 || record_type > 0x25))
    return IHX_BAD_RECORD_TYPE;
    
  if ((record_type == IHX_RECORD_DATA || record_type == IHX_RECORD_READ) && \
      (address < USER_CODE_BASE || address > FLASH_SIZE))
   return IHX_BAD_ADDRESS;
   
  sum = 0;
  i = 0;
  for (i=0; i<byte_count+5; i++) {
    sum += hex8(&line[1 + i*2]);
  }
  
  if (sum != 0)
    return IHX_BAD_CHECKSUM;
    
  return IHX_OK;
}

uint8_t ihx_record_type(char line[]) {
  return hex8(&line[7]);
}

uint8_t ihx_data_byte(char line[], uint8_t n) {
  return hex8(&line[9 + n*2]);
}

void ihx_readline(char line[]) {
  char c;
  uint8_t len;
  
  // Wait for start of record
  while (usb_getchar() != ':') {}
  line[0] = ':';
  
  // Read until newline
  len = 1;
  while (len < (IHX_MAX_LEN*2)+13 && (c = usb_getchar()) != '\n') {
    line[len++] = c;
  }
  line[len+1] = 0;
}

void ihx_write(char line[]) {
  // :ccaaaattxxxxss
  uint8_t byte_count, record_type, i;
  uint16_t address;
  __xdata uint8_t buff[65];
  
  byte_count = hex8(&line[1]);
  address = hex16(&line[3]);
  record_type = hex8(&line[7]);
  
  switch (record_type) {
    case IHX_RECORD_DATA:
      buff[0] = 0xFF; // Padding in case the flash start address is not even.
      for (i=0; i<byte_count; i++)
        buff[i+1] = ihx_data_byte(line, i);
      buff[byte_count+1] = 0xFF; // If there are not an even no. of bytes, pad with 0xFF to preserve flash.
      
      if (address & 1) {
        // Odd start address
        // (byte_count+1)/2 == number of 16-bit words to transfer, rounded up
        flash_check_erase_and_write((uint16_t*)buff, (byte_count+2)/2, address-1);
      } else {
        // Even start address
        // (byte_count+1)/2 == number of 16-bit words to transfer, rounded up
        flash_check_erase_and_write((uint16_t*)(buff+1), (byte_count+1)/2, address);
      }
      
      break;
    case IHX_RECORD_EOF:
      break;
  }

}

