/*
 * CC Bootloader - Flash controller driver
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

#ifndef _FLASH_H_
#define _FLASH_H_

// Flash write timer value:
// FWT = 21000 * FCLK / (16 * 10^9)
// For FCLK = 24MHz, FWT = 0x1F
#define FLASH_FWT 0x1F
// Address of flash controller data register
#define FLASH_FWDATA_ADDR 0xDFAF

void flash_erase_page(uint8_t page);

void flash_write(uint16_t buff[], uint16_t len, uint16_t flash_addr);

// Check if a page was previously erased
uint8_t flash_erased_page(uint8_t page);
// Erase page only if it was never previously erased
void flash_check_and_erase(uint8_t page);
// Write to flash, erasing pages as needed that have never yet been erased
void flash_check_erase_and_write(uint16_t buff[], uint16_t len, uint16_t flash_addr);
// Reset record of which pages have been erased
void flash_reset();
// Erase all user flash pages
void flash_erase_all_user();

#endif // _FLASH_H_