cmake_minimum_required(VERSION 3.21)

set(target lab2)

project(${target})

file(GLOB_RECURSE sources 
    src/*.h
    src/*.cpp
)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")

add_executable(${target} ${sources})

set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

target_link_libraries(${target} boost_system)
