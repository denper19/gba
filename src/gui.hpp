#pragma once

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <SDL.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL_opengles2.h>
#else
#include <SDL_opengl.h>
#endif

#include "ppu.hpp"

class GuiInterface
{
private:

	SDL_WindowFlags window_flags;
	SDL_Window* window{ nullptr };
	SDL_GLContext gl_context;

	bool load_file{ false };
	bool load_bios{ false };
	bool cpu_debug{ false };
	bool ppu_debug{ false };
	bool mem_debug{ false };
	bool reg_debug{ false };

public:
	GuiInterface();

	void GuiMain(Lcd&);

	void CpuDebug();
	void PpuDebug();
	void RegDebug();
	void MemDebug();
	void LoadFile();

	~GuiInterface();
};