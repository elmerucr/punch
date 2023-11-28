#include <cstdio>
#include <cstdint>
#include "common.hpp"
#include "host.hpp"
#include "core.hpp"
#include "debugger.hpp"

int main()
{
	host_t *host = new host_t();
	core_t *core = new core_t();
	debugger_t *debugger = new debugger_t();
	
	core->reset();
	
	bool running = true;
	
	uint32_t frames = 0;
	int32_t cycles = 0;
	
	while (running) {
		frames++;
		
		if (host->events_process_events() == QUIT_EVENT) running = false;
		
		cycles += 985248;
		
		core->run(cycles);
		core->run_blitter();
		
		debugger->redraw();
		
		host->update_textures(&core->blitter->vram[(core->blitter->framebuffer_bank & 0x0f) << 16], &debugger->blitter->vram[(debugger->blitter->framebuffer_bank & 0x0f) << 16]);
		
		host->update_screen();
	}
	
	printf("%i frames\n", frames);
	
	delete debugger;
	delete core;
	delete host;
	
	return 0;
}
