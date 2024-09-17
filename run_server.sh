#!/bin/bash

# This function is used to compile the executables and make them executable
# also makes the test scripts executable
function make_execs() {
    make

    cd tests/
    chmod +x *

    cd ../bin/
    chmod +x *

    cd ../
}

# This function is used to run the server with the given port number, size of queue and number of workers
# if no arguments are given, the default values are 7856, 8 and 5 respectively
function run_server(){
    #if no arguments were given, set $1 to 7856, $2 to 8 and $3 to 5
    if [ $# -eq 0 ]; then
        set -- 7856 8 5
    fi
    #if only one argument was given, set $2 to 8 and $3 to 5
    if [ $# -eq 1 ]; then
        set -- $1 8 5
    fi
    #if only two arguments were given, set $3 to 5
    if [ $# -eq 2 ]; then
        set -- $1 $2 5
    fi

    cd bin/
    ./jobExecutorServer $1 $2 $3
    cd ../
}


make_execs

run_server $1 $2 $3