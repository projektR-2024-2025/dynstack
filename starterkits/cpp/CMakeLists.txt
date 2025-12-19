cmake_minimum_required(VERSION 3.16)
project(buffer_simulator CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Executable
add_executable(buffer_simulator
    src/main.cpp
    src/heuristic.cpp
    ../../simulation/runner.h
    src/SimulatorEvalOp.cpp
    ../../simulation/simulator.cpp
)

# Include paths 
target_include_directories(buffer_simulator PRIVATE
    /usr/local/include
    ../../simulation
)

# Link ECF
target_link_libraries(buffer_simulator /usr/local/lib/libECF.a)

