#
# Kalisto
#
# Copyright (c) 2001-2010
#   Department of Distributed and Dependable Systems
#   Faculty of Mathematics and Physics
#   Charles University, Czech Republic
#
# Kalisto kernel makefile
#

KALISTO_VERSION = 0.9.11

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

CCFLAGS = -march=r4000 -mabi=32 -mgp32 -msoft-float -mlong32 -G 0 -mno-abicalls -fno-pic -fno-builtin -ffreestanding -nostdlib -nostdinc -pipe -Wall -Wextra -Werror -Wno-unused-parameter -Wmissing-prototypes -g3 -std=gnu99 -I. -DKALISTO_VERSION=$(KALISTO_VERSION) -DBUILT_BY=$$USER
ASFLAGS = -march=r4000 -mabi=32 -mgp32 -msoft-float -mlong32 -G 0 -mno-abicalls -fno-pic -fno-builtin -ffreestanding -nostdlib -nostdinc -pipe -Wall -Wextra -Werror -Wno-unused-parameter -Wmissing-prototypes -g3 -std=gnu99 -I. -D__ASM__
LDFLAGS = -G 0 -static -g

### Kernel linker script

LDSFILE = kernel.lds

### Kernel source files

KERNEL_SOURCES = \
	main.c \
	example.c \
	head.S \
	exc/exc.c \
	exc/int.c \
	exc/syscall.c \
	sched/sched.c \
	proc/thread.c \
	proc/sys_thread.c \
	proc/process.c \
	time/timer.c \
	lib/print.c \
	lib/string.c \
	mm/tlb.c \
	mm/falloc.c \
	mm/my_falloc.c \
	mm/malloc.c \
	mm/vmm.c \
	drivers/disk.c \
	drivers/dorder.c \
	drivers/kbd.c \
	synch/mutex.c \
	synch/sys_mutex.c \
	synch/rmutex.c \
	synch/sem.c \
	synch/rwlock.c \
	synch/condvar.c \
	adt/bitmap.c \
	adt/rbtree.c

### If $(KERNEL_TEST) is not empty, add it to kernel sources

ifneq ($(KERNEL_TEST),)
	KERNEL_SOURCES += $(KERNEL_TEST)
	CCFLAGS += -DKERNEL_TEST
endif

### Object, output and temporary files

KERNEL_OBJECTS := $(addsuffix .o,$(basename $(KERNEL_SOURCES)))
KERNEL_RAW = kernel.raw
KERNEL_MAP = kernel.map
KERNEL_BINARY = kernel.bin
KERNEL_DISASM = kernel.disasm

LOADER_SOURCES = boot/loader.S
LOADER_OBJECTS := $(addsuffix .o,$(basename $(LOADER_SOURCES)))
LOADER_RAW = loader.raw
LOADER_MAP = loader.map
LOADER_BINARY = loader.bin
LOADER_DISASM = loader.disasm

DEPEND = Makefile.depend
DEPEND_PREV = $(DEPEND).prev

### Offset header and offset generator

OFFSET_HEADER = include/offset.h

GENERATOR_SOURCE = include/gen_offset.c
GENERATOR_ASM = include/gen_offset.s

### Phony targets

.PHONY: all clean distclean

### Default target

all: $(LOADER_BINARY) $(LOADER_DISASM) $(KERNEL_BINARY) $(KERNEL_DISASM)
	-[ -f $(DEPEND) ] && cp -a $(DEPEND) $(DEPEND_PREV)

### Maintenance

distclean: clean
	rm -f $(KERNEL_BINARY) $(LOADER_BINARY)

clean:
	rm -f $(KERNEL_DISASM) $(KERNEL_MAP) $(KERNEL_RAW) $(LOADER_DISASM) $(LOADER_MAP) $(LOADER_RAW) $(GENERATOR_ASM) $(OFFSET_HEADER) $(DEPEND) $(DEPEND_PREV)
	find . -name '*.o' -exec rm -f \{\} \;

### Dependencies

-include $(DEPEND)

### Disassembly dumps
#
# The disassembly of the compiled kernel and loader might
# be handy in the case you are not sure whether the compiler
# actually produces the code you want in a way you want.
#

$(LOADER_DISASM): $(LOADER_RAW)
	$(OBJDUMP) -d $< > $@

$(KERNEL_DISASM): $(KERNEL_RAW)
	$(OBJDUMP) -d $< > $@

### Binary images
#
# The binary files are made in two stages. First, ELF files are
# created from the object files. Next, the binary files are
# created from the ELF files. The reasons for this are:
#
#  - The ELF files contain debugging information.
#
#  - Linking directly to a binary format does
#    not produce errors on missing symbols.
#

$(LOADER_BINARY): $(LOADER_RAW)
	$(OBJCOPY) -O binary $< $@

$(KERNEL_BINARY): $(KERNEL_RAW)
	$(OBJCOPY) -O binary $< $@

$(LOADER_RAW): $(LDSFILE) $(LOADER_OBJECTS)
	$(LD) $(LDFLAGS) -T $(LDSFILE) -Map $(LOADER_MAP) -o $@ $(LOADER_OBJECTS)

$(KERNEL_RAW): $(LDSFILE) $(KERNEL_OBJECTS)
	$(LD) $(LDFLAGS) -T $(LDSFILE) -Map $(KERNEL_MAP) -o $@ $(KERNEL_OBJECTS)

### Default patterns

%.o: %.obj $(DEPEND)
	cp $< $@

%.o: %.c $(DEPEND)
	$(CC) $(CCFLAGS) -c -o $@ $<

%.o: %.S $(DEPEND)
	$(CC) $(ASFLAGS) -c -o $@ $<

### Source code dependencies

$(DEPEND): $(OFFSET_HEADER)
	-makedepend -f - -- $(CCFLAGS) -- $(KERNEL_SOURCES) > $@ 2> /dev/null
	-[ -f $(DEPEND_PREV) ] && diff -q $(DEPEND_PREV) $@ && mv -f $(DEPEND_PREV) $@

### Offset generator
#
# To make sure that the assembler code is using precise
# offsets to the context_t structure, the offset constants
# are generated from the C code definition of context_t.
#
# The C code is not compiled into binary code, but only
# to assembler code (note the option -S). This assembler
# source code is then parsed by gen_offset.sh.
#

$(OFFSET_HEADER): $(GENERATOR_ASM)
	./include/gen_offset.sh $< > $@

$(GENERATOR_ASM): $(GENERATOR_SOURCE)
	$(CC) $(CCFLAGS) -S -o $@ $<
