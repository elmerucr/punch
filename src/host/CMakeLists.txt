add_library(host STATIC host.cpp)

target_link_libraries(host ${SDL2_LIBRARIES})
