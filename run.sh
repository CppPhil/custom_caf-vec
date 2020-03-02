#!/bin/bash

# Directory containing this bash script
readonly DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

readonly PREV_DIR=$(pwd)

cd $DIR

./build/custom_caf_vec_app --output-file=result.txt --verbosity=2 ./example_log.log

cd $PREV_DIR

exit 0

