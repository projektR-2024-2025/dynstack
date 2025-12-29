#!/bin/bash

rm -rf build
mkdir build/
cd build
cmake ..
make

# cd ..
# ./build/dynstack_trainer parameters.txt