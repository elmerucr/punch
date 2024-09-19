cmake_minimum_required (VERSION 3.5)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -O3")

project(punch)

find_package(SDL2 REQUIRED)

include_directories(
    ${SDL2_INCLUDE_DIRS}
    src/
    src/MC6809/
    src/resid-0.16/
    src/squirrel3/include/
    src/squirrel3/squirrel/
    src/squirrel3/sqstdlib/
)

add_subdirectory(src/)

add_executable(punch src/main.cpp)

# target_link_libraries(punch system ${SDL2_LIBRARIES})
target_link_libraries(punch PRIVATE system SDL2::SDL2-static)
