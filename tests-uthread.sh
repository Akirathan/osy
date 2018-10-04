#! /bin/bash

#
# Compile and boot with tests for assignment 3.
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
    tests/thread/uspace1/test.c \
    tests/thread/thread1/test.c \
    ; do
	emake distclean || fail "Cleanup before compilation"
	emake "USER_TEST=$TEST" || fail "Compilation"
	msim | tee test.log || fail "Execution"
	grep '^Test passed\.\.\.$' test.log > /dev/null || fail "Test $TEST"
	rm -f test.log
	emake distclean || fail "Cleanup after compilation"
done

echo
echo "All tests passed..."
