#
# CC Debugger
# Fergus Noble (c) 2011
#

CC = sdcc

CFLAGS = --model-small --opt-code-speed

LDFLAGS_FLASH = \
	--out-fmt-ihx \
	--code-loc 0x0000 --code-size 0x8000 \
	--xram-loc 0xf000 --xram-size 0x300 \
	--iram-size 0x100

ifdef DEBUG
CFLAGS += --debug
endif

SRC = \
	src/main.c \
	src/usb.c \
	src/intel_hex.c \
	src/usb_descriptors.c 

ADB=$(SRC:.c=.adb)
ASM=$(SRC:.c=.asm)
LNK=$(SRC:.c=.lnk)
LST=$(SRC:.c=.lst)
REL=$(SRC:.c=.rel)
RST=$(SRC:.c=.rst)
SYM=$(SRC:.c=.sym)

PROGS=CCBootloader.hex
PCDB=$(PROGS:.hex=.cdb)
PLNK=$(PROGS:.hex=.lnk)
PMAP=$(PROGS:.hex=.map)
PMEM=$(PROGS:.hex=.mem)
PAOM=$(PROGS:.hex=)

%.rel : %.c
	$(CC) -c $(CFLAGS) -o$*.rel $<

all: $(PROGS)

CCBootloader.hex: $(REL) Makefile
	$(CC) $(LDFLAGS_FLASH) $(CFLAGS) -o CCBootloader.hex $(REL)

clean:
	rm -f $(ADB) $(ASM) $(LNK) $(LST) $(REL) $(RST) $(SYM)
	rm -f $(PROGS) $(PCDB) $(PLNK) $(PMAP) $(PMEM) $(PAOM)

