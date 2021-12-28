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


class GuiInterface
{
private:
	SDL_WindowFlags window_flags;
	SDL_Window* window;
	SDL_GLContext gl_context;
	ImGuiIO& io;
	ImVec4 clear_color;
public:
	GuiInterface();
	void GuiMain();
	void GuiRender();
	~GuiInterface();
};