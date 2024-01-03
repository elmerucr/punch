add_library(app STATIC
	analog.cpp
	blitter.cpp
	core.cpp
	cpu.cpp
	debugger.cpp
	exceptions.cpp
	host.cpp
	keyboard.cpp
	rom.cpp
	sound.cpp
	stats.cpp
	system.cpp
	terminal.cpp
	timer.cpp
)

target_link_libraries(system MC6809 resid)

add_subdirectory(MC6809/)
add_subdirectory(resid-0.16/)
