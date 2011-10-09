#!/bin/bash

source `dirname $0`/colors.sh

ERRORS=0
TESTS=0

for prog in "$@"; do
    RUNNER=`dirname $prog`/`basename $prog`
    NUM_PROGS=`$RUNNER count`
    TESTER_NAME=`$RUNNER name`
    
    for testnum in `eval echo {0..$NUM_PROGS}`; do
        if [ $testnum -ne $NUM_PROGS ]; then
            TEST_NAME=`$RUNNER name $testnum`
            TEST_TYPE=`$RUNNER type $testnum`
            
            ebegin \($TESTER_NAME\) $TEST_NAME
            $RUNNER run $testnum
            RETVAL=$?
            
            if [[ $TEST_TYPE == "NORMALLY_FAILS" ]]; then
                if [[ $RETVAL > 0 ]]; then
                    RETVAL=0
                else
                    RETVAL=1
                fi
            fi
            
            eend $RETVAL noexit
            
            (( TESTS = TESTS + 1))
            if [[ $RETVAL > 0 ]]; then
                (( ERRORS = ERRORS + 1 ))
            fi
        fi
    done
done

echo

if [[ $ERRORS > 0 ]]; then
    echo "=== $ERRORS / $TESTS tests failed ==="
    echo
    exit 1
fi

echo "=== $TESTS tests passed ==="
exit 0