
# adapted from https://gist.github.com/rynr/72734da4b8c7b962aa65

#.PRECIOUS: %.o

MCU     ?= atmega64
ARCH     = avr
PROJECT ?= e004-control

F_CPU   ?= 16000000

PROG     = stk500v2

REV      = REV_C

G++       = avr-g++
GCC       = avr-gcc
OBJCOPY   = avr-objcopy
GCCFLAGS += -DF_CPU=$(F_CPU)UL -I. -I$(LIBDIR)
CXXFLAGS += --stack-auto -mmcu=$(MCU)
LDFLAGS   = -mmcu=$(MCU)

# Files
EXT_C   = c
EXT_C++ = cpp
EXT_ASM = asm

CFLAGS = $(INC)
CFLAGS += -Os
CFLAGS += -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS += -Wall -Wstrict-prototypes
CFLAGS += -DF_CPU=$(F_CPU)UL
CFLAGS += -mmcu=$(MCU)

C++FLAGS = $(INC)
C++FLAGS += -Os -D$(REV)
C++FLAGS += -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
C++FLAGS += -Wall
C++FLAGS += -DF_CPU=$(F_CPU)UL
C++FLAGS += -mmcu=$(MCU)

OBJECTS = \
	$(patsubst %.$(EXT_C),%.o,$(wildcard *.$(EXT_C))) \
	$(patsubst %.$(EXT_C++),%.o,$(wildcard *.$(EXT_C++))) \
	$(patsubst %.$(EXT_ASM),%.o,$(wildcard *.$(EXT_ASM)))

default: $(PROJECT).elf
	echo $(OBJECTS)

%.elf: $(OBJECTS)
	$(GCC) $(CFLAGS) $(OBJECTS) --output $@ $(LDFLAGS)

%.o : %.$(EXT_C)
	$(GCC) $< $(CFLAGS) -c -o $@

%.o : %.$(EXT_C++)
	$(G++) $< $(C++FLAGS) -c -o $@

%.o : %.$(EXT_ASM)
	$(G++) $< $(ASMFLAGS) -c -o $@

clean:
	$(RM) $(PROJECT).elf $(OBJECTS)

flash: $(TARGET)
	@./flash $(PROG) $(MCU) $(TARGET)

flash_opts: $(TARGET)
	@./flash_opts $(PROG) $(MCU) "00FF"

read_eeprom:
	@./read_eeprom $(PROG) $(MCU) eeprom.bin

trash_eeprom:
	@./trash_eeprom $(PROG) $(MCU)

#.PHONY: clean flash flash_opts read_eeprom trash_eeprom
