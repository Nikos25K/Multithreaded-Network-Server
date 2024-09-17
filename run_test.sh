#!/bin/bash

# This function is used to run the test with the given number and port
function run_test(){
    # check that arguments are passed
    if [ $# -lt 1 ]; then
        echo "Usage: run_test <test_number>"
        exit 1
    fi

    # check that the test number is valid
    if [ $1 -lt 1 ] || [ $1 -gt 8 ]; then
        echo "Invalid test number. Must be between 1 and 8"
        exit 1
    fi

    # port number will be checked by the jobCommander

    # run the test
    cd tests/
    ./test_jobExecutor_$1.sh
}


run_test $1