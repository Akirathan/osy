#
# Kalisto
#
# Copyright (c) 2001-2015
#   Department of Distributed and Dependable Systems
#   Faculty of Mathematics and Physics
#   Charles University, Czech Republic
#
# Makefile for user space support
#

### Cross-compiler paths

ifndef CROSS_PREFIX
	CROSS_PREFIX = /usr/local/cross
endif

ARCH = mips32
TARGET = mipsel-linux-gnu

TOOLCHAIN_DIR = $(CROSS_PREFIX)/$(ARCH)

CC = $(TOOLCHAIN_DIR)/bin/$(TARGET)-gcc
LD = $(TOOLCHAIN_DIR)/bin/$(TARGET)-ld
OBJCOPY = $(TOOLCHAIN_DIR)/bin/$(TARGET)-objcopy
OBJDUMP = $(TOOLCHAIN_DIR)/bin/$(TARGET)-objdump

### Compiler, assembler and linker options
#
# All the options are doscribed in detail in the GCC
# and binutils documentation. A brief description of
# some of the specific ones:
#
# -O2                         ... basic optimizations
# -march=r4000                ... generate code for MIPS R4000
# -mabi=32                    ... use standard 32 bit ABI
# -mgp32                      ... general purpose registers are 32 bit
# -msoft-float                ... do not generate floating point instructions
# -mlong32                    ... ints, long and pointers are 32 bit
# -G 0                        ... threshold for putting objects into small data/bss sections
# -mno-abicalls               ... do not generate SVR4-style position independent code
# -fno-pic                    ... do not generate position independent code using $gp
# -fno-builtin                ... do not recognize built-in functions without __builtin_ prefix
# -ffreestanding              ... a compilation without standard library and main()
# -nostdlib                   ... do not look for standard library in system directories
# -nostdinc                   ... do not look for standard header files in system directories
#

CCFLAGS = -O2 -march=r4000 -mabi=32 -mgp32 -msoft-float -mlong32 -G 0 -mno-abicalls -fno-pic -fno-builtin -ffreestanding -nostdlib -nostdinc -pipe -Wall -Wextra -Werror -Wno-unused-parameter -Wmissing-prototypes -g3 -std=gnu99 -I. -I./librt
ASFLAGS = -march=r4000 -mabi=32 -mgp32 -msoft-float -mlong32 -G 0 -mno-abicalls -fno-pic -fno-builtin -ffreestanding -nostdlib -nostdinc -pipe -Wall -Wextra -Werror -Wno-unused-parameter -Wmissing-prototypes -g3 -std=gnu99 -I. -I./librt -D__ASM__
LDFLAGS = -G 0 -static -g

### User space process linker script

LDSFILE = process.lds

### User space process source files

USPACE_SOURCES = \
	main.c

### If $(USER_TEST) is not empty, add it to process sources

ifneq ($(USER_TEST),)
	USPACE_SOURCES += $(USER_TEST)
	CCFLAGS += -DUSER_TEST
endif

### Object, output and temporary files

USPACE_OBJECTS := $(addsuffix .o,$(basename $(USPACE_SOURCES)))
USPACE_RAW = process.raw
USPACE_MAP = process.map
USPACE_BINARY = process.bin
USPACE_DISASM = process.disasm

DEPEND = Makefile.depend
DEPEND_PREV = $(DEPEND).prev

### Phony targets

.PHONY: all clean distclean

### Default target

all: $(USPACE_BINARY) $(USPACE_DISASM)
	-[ -f $(DEPEND) ] && cp -a $(DEPEND) $(DEPEND_PREV)

### Maintenance

distclean: clean
	$(MAKE) -C librt distclean
	rm -f $(USPACE_BINARY)

clean:
	$(MAKE) -C librt clean
	rm -f $(USPACE_DISASM) $(USPACE_MAP) $(USPACE_RAW) $(DEPEND) $(DEPEND_PREV)
	find . -name '*.o' -exec rm -f \{\} \;

### User space run-time library

librt/librt.a:
	$(MAKE) -C librt all

### Dependencies

-include $(DEPEND)

### Disassembly dump
#
# The disassembly of the compiled user space process might
# be handy in the case you are not sure whether the compiler
# actually produces the code you want in a way you want.
#

$(USPACE_DISASM): $(USPACE_RAW)
	$(OBJDUMP) -d $< > $@

### Binary image
#
# The binary file is made in two stages. First, an ELF file is
# created from the object files. Next, the binary file is
# created from the ELF file. The reasons for this are:
#
#  - The ELF file contains debugging information.
#
#  - Linking directly to a binary format does
#    not produce errors on missing symbols.
#

$(USPACE_BINARY): $(USPACE_RAW)
	$(OBJCOPY) -O binary $< $@

$(USPACE_RAW): $(LDSFILE) $(USPACE_OBJECTS) librt/librt.a
	$(LD) $(LDFLAGS) -T $(LDSFILE) -Map $(USPACE_MAP) -o $@ $(USPACE_OBJECTS) librt/librt.a

### Default patterns

%.o: %.c $(DEPEND)
	$(CC) $(CCFLAGS) -c -o $@ $<

%.o: %.S $(DEPEND)
	$(CC) $(ASFLAGS) -c -o $@ $<

### Source code dependencies

$(DEPEND):
	-makedepend -f - -- $(CCFLAGS) -- $(USPACE_SOURCES) > $@ 2> /dev/null
	-[ -f $(DEPEND_PREV) ] && diff -q $(DEPEND_PREV) $@ && mv -f $(DEPEND_PREV) $@
