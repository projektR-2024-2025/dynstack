cmake_minimum_required(VERSION 3.19)

if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE
      "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
      CACHE STRING "")
  message(" [INFO] Using vcpkg toolchain from: ${CMAKE_TOOLCHAIN_FILE}")
endif()

project(dynstack)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Protobuf REQUIRED)
find_package(cppzmq CONFIG REQUIRED)
find_package(absl CONFIG REQUIRED)

add_executable(stacking
  src/stacking.cc

  src/hotstorage/heuristic.h
  src/hotstorage/custom_heuristic_dj.cc
  src/hotstorage/hotstorage_model.pb.cc
  src/hotstorage/hotstorage_model.pb.h

  src/rollingmill/heuristic.h
  src/rollingmill/heuristic.cc
  src/rollingmill/rollingmill_model.pb.cc
  src/rollingmill/rollingmill_model.pb.h

  src/cranescheduling/heuristic.h
  src/cranescheduling/heuristic.cc
  src/cranescheduling/cranescheduling_model.pb.cc
  src/cranescheduling/cranescheduling_model.pb.h
)

target_include_directories(stacking PRIVATE
  ${Protobuf_INCLUDE_DIRS}
  /usr/local/include        # ECF
)

target_link_libraries(stacking PRIVATE
  protobuf::libprotoc
  protobuf::libprotobuf
  protobuf::libprotobuf-lite

  cppzmq
  cppzmq-static

  absl::base
  absl::strings
  absl::status
  absl::statusor
  absl::log_internal_check_op
  absl::log_internal_message
  absl::log_internal_format

  ECF
)
