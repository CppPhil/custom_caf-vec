#!/bin/bash

# Directory containing this bash script
readonly DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

readonly PREV_DIR=$(pwd)

cd $DIR

file=$(ls ../caf_shiviz_trial/build/*.log | head -1)

./build/custom_caf_vec_app --output-file=result.txt --verbosity=2 $file

cd $PREV_DIR

exit 0

