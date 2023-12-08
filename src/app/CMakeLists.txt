add_library(app STATIC app.cpp)

target_link_libraries(app host core keyboard)
