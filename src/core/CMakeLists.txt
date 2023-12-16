add_library(core STATIC core.cpp)

target_link_libraries(core cpu blitter debugger timer)
