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

#include "cc1111.h"
#include "flash.h"
#include "main.h"
#include "usb.h"

static __xdata struct cc_dma_channel dma0_config;
uint32_t erased_page_flags = 0;

void flash_erase_page(uint8_t page) {
  // Don't let's erase the bootloader, please
  if (page < USER_FIRST_PAGE)
    return;
  
  // Waiting for the flash controller to be ready
  while (FCTL & FCTL_BUSY) {}
  
  // Set bit showing that the flash page has been erased
  erased_page_flags |= ((uint32_t)1 << page);
  
  // Configure flash controller for a flash page erase
  // FADDRH[5:1] contains the page to erase
  // FADDRH[1]:FADDRL[7:0] contains the address within the page
  // (16-bit word addressed)
  FWT = FLASH_FWT;
  FADDRH = page << 1;
  FADDRL = 0x00;

  // Erase the page that will be written to
  FCTL |=  FCTL_ERASE;
  nop(); // Required, see datasheet
  
  // Wait for the erase operation to complete
  while (FCTL & FCTL_BUSY) {}
  
  //FCTL &=  ~FCTL_ERASE;
}

void flash_write_trigger() {
  // Enable flash write. Generates a DMA trigger. Must be aligned on a 2-byte
  // boundary and is therefore implemented in assembly.
  
  // p.s. if this looks a little crazy its because it is, sdcc doesn't currently
  // support explicitly specifying code alignment which would make this easier
  
  __asm
    .globl flash_write_trigger_instruction
    .globl flash_write_trigger_done
    
    ; Put our trigger instruction in the HOME segment (shared with some startup code)
    ; where it wont move around too much
    .area HOME (CODE)
    ; Comment or uncomment these lines to adjust if you change the start.asm code
    nop               ; Padding to get onto 16-bit boundary
  flash_write_trigger_instruction:
    orl _FCTL, #0x02  ; FCTL |=  FCTL_ERASE
    nop               ; Required, see datasheet.
    ljmp flash_write_trigger_done ; Jump back into our function
    
    ; Meanwhile, back in the main CSEG segment...
    .area CSEG (CODE)
    ; Jump to the trigger instruction
    ljmp flash_write_trigger_instruction
  flash_write_trigger_done:
  __endasm;
}

void flash_write(uint16_t buff[], uint16_t len, uint16_t flash_addr) {
  // NOTE: len is the number of 16-bit words to transfer

  // Setup DMA descriptor
  dma0_config.src_high  = ((uint16_t)buff >> 8) & 0x00FF;
  dma0_config.src_low   = (uint16_t)buff & 0x00FF;
  dma0_config.dst_high  = (FLASH_FWDATA_ADDR >> 8) & 0x00FF;
  dma0_config.dst_low   = FLASH_FWDATA_ADDR & 0x00FF;
  dma0_config.len_high  = DMA_LEN_HIGH_VLEN_LEN;
  dma0_config.len_high |= ((len*2) >> 8) & DMA_LEN_HIGH_MASK;
  dma0_config.len_low   = (len*2) & 0x00FF;
  
  dma0_config.cfg0 = \
    DMA_CFG0_WORDSIZE_8 | \
    DMA_CFG0_TMODE_SINGLE | \
    DMA_CFG0_TRIGGER_FLASH;
  
  dma0_config.cfg1 = \
    DMA_CFG1_SRCINC_1 | \
    DMA_CFG1_DESTINC_0 | \
    DMA_CFG1_PRIORITY_HIGH;
  
  // Point DMA controller at our DMA descriptor
  DMA0CFGH = ((uint16_t)&dma0_config >> 8) & 0x00FF;
  DMA0CFGL = (uint16_t)&dma0_config & 0x00FF;

  // Waiting for the flash controller to be ready
  while (FCTL & FCTL_BUSY) {}

  // Configure the flash controller
  FWT = FLASH_FWT;
  FADDRH = (flash_addr >> 9) & 0x3F;
  FADDRL = (flash_addr >> 1) & 0xFF;

  // Arm the DMA channel, so that a DMA trigger will initiate DMA writing
  DMAARM |= DMAARM_DMAARM0;

  // Enable flash write - triggers the DMA transfer
  flash_write_trigger();
  
  // Wait for DMA transfer to complete
  while (!(DMAIRQ & DMAIRQ_DMAIF0)) {}

  // Wait until flash controller not busy
  while (FCTL & (FCTL_BUSY | FCTL_SWBSY)) {}
  
  // By now, the transfer is completed, so the transfer count is reached.
  // The DMA channel 0 interrupt flag is then set, so we clear it here.
  DMAIRQ &= ~DMAIRQ_DMAIF0;
}

uint8_t flash_erased_page(uint8_t page) {
  // Check if a page was previously erased
  if (erased_page_flags & ((uint32_t)1 << page))
    return 1;
  else
    return 0;
}

void flash_check_and_erase(uint8_t page) {
  // Erase page only if it was never previously erased
  if (!flash_erased_page(page))
    flash_erase_page(page);
}

void flash_check_erase_and_write(uint16_t buff[], uint16_t len, uint16_t flash_addr) {
  uint8_t i, start_page, end_page;
  
  start_page = flash_addr / 1024;
  end_page = (flash_addr + len) / 1024;
  
  // Check and erase pages in range
  for (i=start_page; i<=end_page; i++)
    flash_check_and_erase(i);
  
  flash_write(buff, len, flash_addr);
}

void flash_reset() {
  erased_page_flags = 0;
}

void flash_erase_all_user() {
  // Erase all user flash pages
  uint8_t i;
  for (i=USER_FIRST_PAGE; i<FLASH_PAGES; i++)
    flash_erase_page(i);
}

