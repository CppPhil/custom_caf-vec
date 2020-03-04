#!/bin/bash

function catch_errors() {
    printf "\nbuild.sh failed!\n" >&2
    exit 1
}

trap catch_errors ERR;

# Directory containing this bash script
readonly DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

readonly PREV_DIR=$(pwd)

cd $DIR

build_type="$1"

if [ -z "$build_type" ]; then
    build_type="Release"
fi

if [ ! -d build ]; then
    mkdir build
fi

cd build
cmake -DCMAKE_BUILD_TYPE=$build_type -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -G "Unix Makefiles" ..
cmake --build . -- -j$(nproc)

cd $PREV_DIR

exit 0

