#
# Kalisto
#
# Copyright (c) 2001-2018
#   Department of Distributed and Dependable Systems
#   Faculty of Mathematics and Physics
#   Charles University, Czech Republic
#
# Kalisto primary makefile. The defalt target ("all")
# build the kernel and user space (currently not
# implemented). The target "doc" can be used to create
# Doxygen documentation. Targets "clean" and "distclean"
# can be used to cleanup the source tree.
#

KALISTO_VERSION = 0.9.12
DISTNAME = kalisto-$(KALISTO_VERSION)

### Phony targets

.PHONY: all clean distclean kernel user doc

### Default target

all: kernel user

kernel:
	$(MAKE) -C kernel KERNEL_TEST="$(KERNEL_TEST)"

user:
	$(MAKE) -C user USER_TEST="$(USER_TEST)"

doc:
	$(MAKE) -C doc

clean:
	$(MAKE) -C kernel clean
	$(MAKE) -C user clean
	$(MAKE) -C doc clean

distclean:
	$(MAKE) -C kernel distclean
	$(MAKE) -C user distclean
	$(MAKE) -C doc distclean

dist:
	mkdir $(DISTNAME)
	cp Makefile README msim.conf $(EXTRA_DIST_FILES) ddisk.img $(DISTNAME)
	cp tests*.sh $(DISTNAME)
	cp -RL contrib doc kernel user $(DISTNAME)
	make -C $(DISTNAME) distclean >/dev/null 2>/dev/null
	tar cjf $(DISTNAME).tar.bz2 $(DISTNAME)
	rm -rf $(DISTNAME)

