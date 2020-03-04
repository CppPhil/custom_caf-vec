#!/bin/bash

function catch_errors() {
    printf "\nrun.sh failed!\n" >&2
    exit 1
}

trap catch_errors ERR;

# Directory containing this bash script
readonly DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

readonly PREV_DIR=$(pwd)

cd $DIR

./build/custom_caf_vec_app --output-file=result.txt --verbosity=2 --include-hidden-actors ./example_log.log

cd $PREV_DIR

exit 0

