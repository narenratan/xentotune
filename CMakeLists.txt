cmake_minimum_required(VERSION 3.9.6...3.15.0)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.15 CACHE STRING "Build for 10.15")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

set(PLUGIN Xentotune)
set(TESTS tests)

project(PLUGIN LANGUAGES CXX)

# Server CLAP plugin

add_library(
    ${PLUGIN}
    MODULE
    src/plugin.cpp
    libs/MTS-ESP/Client/libMTSClient.cpp
)

set_target_properties(${PLUGIN} PROPERTIES SUFFIX ".clap" PREFIX "")

target_include_directories(
    ${PLUGIN}
    PUBLIC
    libs
)

add_executable(
    ${TESTS}
    tests/tests.cpp
    libs/MTS-ESP/Client/libMTSClient.cpp
)

target_include_directories(
    ${TESTS}
    PUBLIC
    src
    libs
)
