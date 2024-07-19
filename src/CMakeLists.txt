add_library(system STATIC
	analog.cpp
	blitter.cpp
	core.cpp
	cpu.cpp
	debugger.cpp
	exceptions.cpp
	host.cpp
	keyboard.cpp
	moon.cpp
	rom.cpp
	sound.cpp
	stats.cpp
	system.cpp
	terminal.cpp
	timer.cpp
)

target_link_libraries(system lua MC6809 resid)

add_subdirectory(lua-5.4.7/src/)
add_subdirectory(MC6809/)
add_subdirectory(resid-0.16/)
