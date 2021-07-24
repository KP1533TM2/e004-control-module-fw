
# adapted from https://gist.github.com/rynr/72734da4b8c7b962aa65

.PHONY: clean flash flash_fuses hex disasm size
.PRECIOUS: %.o %.elf
.DEFAULT_GOAL := help

PROJECT ?= e004-control

MCU     ?= atmega64
ARCH     = avr

# AVR fuses in AVRDUDE format:
HFUSE    = 0xC1
LFUSE    = 0xAF
EFUSE    = 0xFF

F_CPU   ?= 16000000

AVRDUDE  = avrdude
PORT     = /dev/ttyUSB0
PROG     = stk500v2

REV      = REV_C

G++       = avr-g++
GCC       = avr-gcc
OBJCOPY   = avr-objcopy
OBJDUMP   = avr-objdump
SIZE      = avr-size
RM       = rm -f

LDFLAGS   = -mmcu=$(MCU)

GIT_VERSION := "$(shell git describe --abbrev=8 --dirty --always --tags)"

# Output files
ELF_FILE  = $(PROJECT).elf
HEX_FILE  = $(PROJECT).hex

# Files
EXT_C   = c
EXT_C++ = cpp
EXT_ASM = asm

CFLAGS = $(INC)
CFLAGS += -Os
CFLAGS += -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS += -Wall -Wstrict-prototypes -lm
CFLAGS += -DF_CPU=$(F_CPU)UL
CFLAGS += -mmcu=$(MCU)

C++FLAGS = $(INC)
C++FLAGS += -Os -D$(REV)
C++FLAGS += -DEXTRA_VER=\"$(GIT_VERSION)\"
C++FLAGS += -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
C++FLAGS += -Wall -lm
C++FLAGS += -DF_CPU=$(F_CPU)UL
C++FLAGS += -mmcu=$(MCU)

OBJECTS = \
	$(patsubst %.$(EXT_C),%.o,$(wildcard *.$(EXT_C))) \
	$(patsubst %.$(EXT_C++),%.o,$(wildcard *.$(EXT_C++))) \
	$(patsubst %.$(EXT_ASM),%.o,$(wildcard *.$(EXT_ASM)))

hex: $(ELF_FILE) size
	$(OBJCOPY) -O ihex $(PROJECT).elf $(PROJECT).hex

size: $(ELF_FILE)
	$(SIZE) -C -x --mcu=$(MCU) $(ELF_FILE)

disasm: $(ELF_FILE)
	$(OBJDUMP) -D $(PROJECT).elf > $(PROJECT).disasm && \
	less $(PROJECT).disasm

$(ELF_FILE): $(OBJECTS)
	$(GCC) $(CFLAGS) $(OBJECTS) --output $@ $(LDFLAGS)

%.o : %.$(EXT_C)
	$(GCC) $< $(CFLAGS) -c -o $@

%.o : %.$(EXT_C++)
	$(G++) $< $(C++FLAGS) -c -o $@

%.o : %.$(EXT_ASM)
	$(G++) $< $(ASMFLAGS) -c -o $@

clean:
	$(RM) $(ELF_FILE) $(HEX_FILE) $(OBJECTS)

flash: hex
	$(AVRDUDE) -c $(PROG) -p $(MCU) -P $(PORT) -U flash:w:$(HEX_FILE):i

flash_fuses:
	$(AVRDUDE) -c $(PROG) -p $(MCU) -P $(PORT) \
	-U lfuse:w:$(LFUSE):m -U hfuse:w:$(HFUSE):m -U efuse:w:$(EFUSE):m 
	

help:
	@echo "usage:"
	@echo "  make <target> [PORT=... PROG=...]"
	@echo ""
	@echo "targets:"
	@echo ""
	@echo "  clean        Remove any non-source files"
	@echo ""
	@echo "  config       Shows the current configuration"
	@echo ""
	@echo "  help         Shows this help"
	@echo ""
	@echo "  hex          Produce Intel Hex file"
	@echo ""
	@echo "  size         Show memory usage"
	@echo ""
	@echo "  disasm       Disassemble ELF (intermediate) binary"
	@echo ""
	@echo "  flash        Upload firmware to AVR through programmer"
	@echo "               Default programmer is $(PROG),"
	@echo "               default port is $(PORT);"
	@echo "               Use PROG= and PORT= to specify different ones."
	@echo "               For available programmer options check your"
	@echo "               avrdude installation."
	@echo ""
	@echo "  flash_fuses  Program fuses."
	@echo "               Options and defaults are same as for 'flash' target."
	@echo ""
