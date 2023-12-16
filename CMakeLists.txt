cmake_minimum_required (VERSION 3.0)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -O3")

project(punch)

find_package(sdl2 REQUIRED)

include_directories(
    ${SDL2_INCLUDE_DIRS}
    src/
    src/app/
    src/blitter/
    src/core/
    src/debugger/
    src/host/
    src/keyboard/
    src/MC6809/
    src/terminal/
    src/timer/
)

add_subdirectory(src/)

add_executable(punch src/cpu.cpp src/exceptions.cpp src/main.cpp)

target_link_libraries(punch app)
