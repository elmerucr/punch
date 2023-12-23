add_library(app STATIC
	analog.cpp
	app.cpp
	blitter.cpp
	core.cpp
	cpu.cpp
	debugger.cpp
	exceptions.cpp
	host.cpp
	keyboard.cpp
	rom.cpp
	sound.cpp
	terminal.cpp
	timer.cpp
)

target_link_libraries(app MC6809 resid)

add_subdirectory(MC6809/)
add_subdirectory(resid-0.16/)
