cmake_minimum_required(VERSION 3.21)
project(Teleme VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-pthread -O0 -g -w")

include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}
        # ${CMAKE_CURRENT_SOURCE_DIR}/other/dirs
)

add_library(
        mylib
        STATIC

        # Sources
        ${CMAKE_CURRENT_SOURCE_DIR}/fake_receiver.c
)

target_link_libraries(mylib
        # Other librs
        )

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

add_executable(Teleme main.cpp)
target_link_libraries(Teleme mylib)