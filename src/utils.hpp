#pragma once
#include "bus.hpp"
#include "cpu.hpp"
#include "ppu.hpp"
#include "imgui.h"
#include "imgui-SFML.h"
#include "imgui_memory_editor.h"

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/system/Clock.hpp>
#include <SFML/Window/Event.hpp>


class Gui
{
private:
	sf::RenderWindow window;
	sf::Texture screen;
	int user_mode;
	int arm_mode;
	sf::Color bgColor;
	sf::Clock deltaClock;
	bool load_rom = false;
	bool show_display = false;
	bool cpu_debug_window = false;
	bool ppu_debug_window = false;
	bool memory_window = false;
	bool save_state = false;
	bool load_state = false;
public:
	void MainWindow(Arm& cpu, Bus& bus, Lcd& ppu);
	void CpuDebugWindow(Arm& cpu, Lcd& ppu, Bus& bus);
	void PpuDebugWindow(Lcd& ppu);
	void MemoryWindow(Bus& bus);
	void writeRegisters(Arm& cpu);
	void ShowDisplay(Lcd& ppu);
	void LoadRom(Arm& cpu);
	void SaveState(Arm& cpu);
	void LoadState(Arm& cpu);
	void Rewind(Arm& cpu);
	void initImguiSfml();
	void imguiMainMenuBar();
};

