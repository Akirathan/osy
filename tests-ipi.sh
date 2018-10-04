#! /bin/bash

#
# Compile and boot with tests for assignment 1.
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
    tests/ipi/ipi1/test.c \
    ; do
	emake distclean || fail "Cleanup before compilation"
	emake "KERNEL_TEST=$TEST" || fail "Compilation"
	msim
	emake distclean || fail "Cleanup after compilation"
done

echo
echo "All tests passed..."
