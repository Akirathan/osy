#! /bin/bash

#
# Kalisto
#
# Copyright (c) 2001-2015
#   Department of Distributed and Dependable Systems
#   Faculty of Mathematics and Physics
#   Charles University, Czech Republic
#
# Compile and boot with tests for semaphores.
# The correct result of each test is signaled by
#
# Test passed...
#

fail() {
	rm -f test.log
	echo
	echo "Failure: $1"
	exit 1
}

# Don't output command executed by make unless run with -v
if [ "$1" == "-v" ] ; then
	SILENT_MAKE=""
else
	SILENT_MAKE="--silent"
fi

emake() {
	echo "Running make $SILENT_MAKE $@"
	make $SILENT_MAKE "$@"
}

for TEST in \
    tests/condvar/condvar1/test.c \
    tests/condvar/condvar2/test.c \
    ; do
	emake distclean || fail "Cleanup before compilation"
	emake "KERNEL_TEST=$TEST" || fail "Compilation"
	msim | tee test.log || fail "Execution"
	grep '^Test passed\.\.\.$' test.log > /dev/null || fail "Test $TEST"
	rm -f test.log
	emake distclean || fail "Cleanup after compilation"
done

echo
echo "All tests passed..."
