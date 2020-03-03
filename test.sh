#!/bin/bash

# Directory containing this bash script
readonly DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

readonly PREV_DIR=$(pwd)

cd $DIR

./build.sh 

./run.sh

if [ -z $(diff old_result.txt result.txt) ]; then
    printf "\nSUCCESS: 1 out of 1 tests executed successfully.\n"
else
    printf "\nFAILURE: old_result.txt and result.txt differ!\n"
fi

cd $PREV_DIR

exit 0

