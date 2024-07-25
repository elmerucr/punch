add_library(system STATIC
	analog.cpp
	blitter.cpp
	commander.cpp
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

target_link_libraries(system lua MC6809 resid squirrel sqstd)

add_subdirectory(lua-5.4.7/src/)
add_subdirectory(squirrel3/squirrel/)
add_subdirectory(squirrel3/sqstdlib/)
add_subdirectory(MC6809/)
add_subdirectory(resid-0.16/)
