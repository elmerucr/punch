cmake_minimum_required (VERSION 3.0)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -O3")

project(punch)

find_package(sdl2 REQUIRED)

include_directories(
    ${SDL2_INCLUDE_DIRS}
    src/
    src/lua-5.4.7/src/
    src/MC6809/
    src/resid-0.16/
)

add_subdirectory(src/)

add_executable(punch src/main.cpp)

target_link_libraries(punch system ${SDL2_LIBRARIES})
