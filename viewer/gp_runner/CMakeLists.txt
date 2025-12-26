cmake_minimum_required(VERSION 3.19)

set(CMAKE_CXX_STANDARD 17) 

project(dynstack)

include(FindProtobuf)
find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIR})
find_package(cppzmq CONFIG REQUIRED)

add_executable(stacking
  "src/stacking.cc"
  "src/hotstorage/heuristic.h"
  "src/hotstorage/heuristic.cc"
  "src/hotstorage/hotstorage_model.pb.cc"
  "src/hotstorage/hotstorage_model.pb.h"
  "src/SimulatorEvalOp.cpp"
)

target_include_directories(stacking PRIVATE /usr/local/include)
target_link_libraries(stacking PRIVATE /usr/local/lib/libECF.a)

target_link_libraries(stacking PRIVATE protobuf::libprotobuf)
target_link_libraries(stacking PRIVATE cppzmq)
target_link_libraries(stacking PRIVATE ECF)