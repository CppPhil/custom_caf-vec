#!/bin/bash

function catch_errors() {
    printf "\ntest.sh failed!\n" >&2
    exit 1 
}

trap catch_errors ERR;

# Directory containing this bash script
readonly DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

readonly PREV_DIR=$(pwd)

set -e

cd $DIR

./format.sh

./build.sh

./run.sh

if [ -z $(diff old_result.txt result.txt) ]; then
    printf "\nSUCCESS: 1 out of 1 tests executed successfully.\n"
else
    printf "\nFAILURE: old_result.txt and result.txt differ!\n"
fi

cd $PREV_DIR

exit 0

