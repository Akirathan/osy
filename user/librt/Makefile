#
# Kalisto
#
# Copyright (c) 2001-2015
#   Department of Distributed and Dependable Systems
#   Faculty of Mathematics and Physics
#   Charles University, Czech Republic
#
# Makefile for user space run-time library
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
AR = $(TOOLCHAIN_DIR)/bin/$(TARGET)-ar

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

CCFLAGS = -O2 -march=r4000 -mabi=32 -mgp32 -msoft-float -mlong32 -G 0 -mno-abicalls -fno-pic -fno-builtin -ffreestanding -nostdlib -nostdinc -pipe -Wall -Wextra -Werror -Wno-unused-parameter -Wmissing-prototypes -g3 -std=gnu99 -I.
ASFLAGS = -march=r4000 -mabi=32 -mgp32 -msoft-float -mlong32 -G 0 -mno-abicalls -fno-pic -fno-builtin -ffreestanding -nostdlib -nostdinc -pipe -Wall -Wextra -Werror -Wno-unused-parameter -Wmissing-prototypes -g3 -std=gnu99 -I. -D__ASM__

### Run-time library source files

LIBRT_SOURCES = \
	entry.S \
	librt.c \
	malloc.c \
	thread.c \
	mutex.c \
	stdio.c

### Object, output and temporary files

LIBRT_OBJECTS := $(addsuffix .o,$(basename $(LIBRT_SOURCES)))
LIBRT_TARGET = librt.a

DEPEND = Makefile.depend
DEPEND_PREV = $(DEPEND).prev

### Phony targets

.PHONY: all clean distclean

### Default target

all: $(LIBRT_TARGET)
	-[ -f $(DEPEND) ] && cp -a $(DEPEND) $(DEPEND_PREV)

### Maintenance

distclean: clean
	rm -f $(LIBRT_TARGET)

clean:
	rm -f $(LIBRT_TARGET) $(DEPEND) $(DEPEND_PREV)
	find . -name '*.o' -exec rm -f \{\} \;

### Dependencies

-include $(DEPEND)

### Run-time library archive

$(LIBRT_TARGET): $(LIBRT_OBJECTS)
	$(AR) rc $(LIBRT_TARGET) $(LIBRT_OBJECTS)

### Default patterns

%.o: %.obj $(DEPEND)
	cp $< $@

%.o: %.c $(DEPEND)
	$(CC) $(CCFLAGS) -c -o $@ $<

%.o: %.S $(DEPEND)
	$(CC) $(ASFLAGS) -c -o $@ $<

### Source code dependencies

$(DEPEND):
	-makedepend -f - -- $(CCFLAGS) -- $(USPACE_SOURCES) > $@ 2> /dev/null
	-[ -f $(DEPEND_PREV) ] && diff -q $(DEPEND_PREV) $@ && mv -f $(DEPEND_PREV) $@
