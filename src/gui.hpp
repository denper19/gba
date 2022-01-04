#pragma once
#include <algorithm>
#include <array>
#include <mutex>
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

#include "bus.hpp"
#include "ppu.hpp"

class GuiInterface
{
private:

	SDL_WindowFlags window_flags;
	SDL_Window* window{ nullptr };
	SDL_GLContext gl_context;

	std::mutex mu;

	bool load_file{ false };
	bool load_bios{ false };
	bool cpu_debug{ false };
	bool ppu_debug{ false };
	bool mem_debug{ false };
	bool reg_debug{ false };
	bool disp_game{ false };
public:
	GuiInterface();

	void GuiMain(Lcd*, Bus*);

	void ShowGame(Bus* bus, Lcd* ppu, unsigned int id);
	void CpuDebug();
	void PpuDebug();
	void RegDebug();
	void MemDebug();
	void LoadFile();

	~GuiInterface();
};