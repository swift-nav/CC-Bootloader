#
# CC Debugger
# Fergus Noble (c) 2011
#

CC = sdcc
AS = sdas8051

CFLAGS = --model-small --opt-code-size

LDFLAGS_FLASH = \
	--out-fmt-ihx \
	--code-loc 0x0000 --code-size 0x1400 \
	--xram-loc 0xf000 --xram-size 0x300 \
	--iram-size 0x100

ASFLAGS = -plosgff

ifdef DEBUG
CFLAGS += --debug
endif

SRC = \
	src/main.c \
	src/usb.c \
	src/flash.c \
	src/intel_hex.c \
	src/usb_descriptors.c 

ASM_SRC = src/start.asm

ADB = $(SRC:.c=.c.adb)
ASM = $(SRC:.c=.c.asm)
LNK = $(SRC:.c=.c.lnk)
LST = $(SRC:.c=.c.lst)
REL = $(SRC:.c=.c.rel)
RST = $(SRC:.c=.c.rst)
SYM = $(SRC:.c=.c.sym)

ASM_ADB = $(ASM_SRC:.asm=.adb)
ASM_LNK = $(ASM_SRC:.asm=.lnk)
ASM_LST = $(ASM_SRC:.asm=.lst)
ASM_REL = $(ASM_SRC:.asm=.rel)
ASM_RST = $(ASM_SRC:.asm=.rst)
ASM_SYM = $(ASM_SRC:.asm=.sym)

PROGS = CCBootloader.hex
PCDB = $(PROGS:.hex=.cdb)
PLNK = $(PROGS:.hex=.lnk)
PMAP = $(PROGS:.hex=.map)
PMEM = $(PROGS:.hex=.mem)
PAOM = $(PROGS:.hex=)

%.c.rel : %.c
	$(CC) -c $(CFLAGS) -o$*.c.rel $<

%.rel : %.asm
	$(AS) -c $(ASFLAGS) $<

all: $(PROGS)

CCBootloader.hex: $(REL) $(ASM_REL) Makefile
	$(CC) $(LDFLAGS_FLASH) $(CFLAGS) -o CCBootloader.hex $(ASM_REL) $(REL)

clean:
	rm -f $(ADB) $(ASM) $(LNK) $(LST) $(REL) $(RST) $(SYM)
	rm -f $(ASM_ADB) $(ASM_LNK) $(ASM_LST) $(ASM_REL) $(ASM_RST) $(ASM_SYM)
	rm -f $(PROGS) $(PCDB) $(PLNK) $(PMAP) $(PMEM) $(PAOM)

