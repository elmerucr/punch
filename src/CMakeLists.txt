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

add_library(squirrel STATIC
	squirrel3/squirrel/sqapi.cpp
	squirrel3/squirrel/sqbaselib.cpp
	squirrel3/squirrel/sqclass.cpp
	squirrel3/squirrel/sqcompiler.cpp
	squirrel3/squirrel/sqdebug.cpp
	squirrel3/squirrel/sqfuncstate.cpp
	squirrel3/squirrel/sqlexer.cpp
	squirrel3/squirrel/sqmem.cpp
	squirrel3/squirrel/sqobject.cpp
	squirrel3/squirrel/sqstate.cpp
	squirrel3/squirrel/sqtable.cpp
	squirrel3/squirrel/sqvm.cpp
)

add_library(sqstd STATIC
	squirrel3/sqstdlib/sqstdaux.cpp
	squirrel3/sqstdlib/sqstdblob.cpp
	squirrel3/sqstdlib/sqstdio.cpp
	squirrel3/sqstdlib/sqstdmath.cpp
	squirrel3/sqstdlib/sqstdrex.cpp
	squirrel3/sqstdlib/sqstdstream.cpp
	squirrel3/sqstdlib/sqstdstring.cpp
	squirrel3/sqstdlib/sqstdsystem.cpp
)

target_link_libraries(system lua MC6809 resid squirrel sqstd)

add_subdirectory(lua-5.4.7/src/)
add_subdirectory(MC6809/)
add_subdirectory(resid-0.16/)
