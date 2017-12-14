cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)

project(usertool CXX)

set(sequentialioSRC
    sequentialio.cc)

add_executable(sequentialio ${sequentialioSRC})

SET( CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -Wall )

set_target_properties(sequentialio PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)
