#!/bin/bash

# Directory containing this bash script
readonly DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

readonly PREVIOUS_DIRECTORY=$(pwd)

readonly ROOT_DIR=$DIR/..

cd $ROOT_DIR/external

cd ./actor-framework

if [ ! -d ./build ] || [ "$1" == "rebuild" ]; then
    rm -rf ./build
    
    ./configure --no-examples --no-python --no-unit-tests --with-log-level=TRACE --with-actor-profiler

    cd ./build

    make -j$(nproc)

    cd ..
fi

cd $PREVIOUS_DIRECTORY

exit 0

