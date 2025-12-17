#!/bin/bash

#regenerate model classes
cd ./starterkits
#protoc hotstorage_model.proto --cpp_out=cpp/src/hotstorage
#protoc rollingmill_model.proto --cpp_out=cpp/src/rollingmill 
#protoc cranescheduling_model.proto --cpp_out=cpp/src/cranescheduling

cd ./cpp
echo 'Building solver...'
cmake -S . -B build
cmake --build build

cd build
echo 'Running solver...'
./stacking tcp://127.0.0.1:2222 658f9b28-6686-40d2-8800-611bd8466215 HS
#./stacking <url> <id> <sim-type>
