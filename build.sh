#!/bin/bash

# Directory containing this bash script
readonly DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

readonly PREV_DIR=$(pwd)

cd $DIR

build_type="$1"

if [ -z "$build_type" ]; then
    build_type="Release"
fi

mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=$build_type -G "Unix Makefiles" ..
cmake --build . -- -j$(nproc)

cd $PREV_DIR

exit 0

