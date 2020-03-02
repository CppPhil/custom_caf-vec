#!/bin/bash

# Directory containing this bash script
readonly DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

readonly PREV_DIR=$(pwd)

cd $DIR

diff old_result.txt result.txt 

cd $PREV_DIR

exit 0

