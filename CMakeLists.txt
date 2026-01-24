cmake_minimum_required(VERSION 3.16)
project(dynstack_trainer CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(dynstack_trainer
    trainer/src/main.cpp
    trainer/src/CustomHeuristic.cpp
    trainer/src/PriorityHeuristic.cpp
    trainer/src/SimulatorEvalOp.cpp
    trainer/simulator/Simulator.cpp
    trainer/src/Model.cpp
    trainer/src/SimulatorEvalCGPOp.cpp
)

target_include_directories(dynstack_trainer PRIVATE
    trainer/src
    trainer/simulator
)

target_link_libraries(dynstack_trainer PRIVATE ECF)