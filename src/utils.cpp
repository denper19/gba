#include "utils.hpp"

static MemoryEditor mem_edit;

void Gui::writeRegisters(Arm& cpu)
{
	if (user_mode == 7)
	{
		ImGui::Text("SPSR_fiq: 0x%002X", cpu.spsr[0]);
		ImGui::Text("SPSR_irq: 0x%002X", cpu.spsr[1]);
		ImGui::Text("SPSR_sys: 0x%002X", cpu.spsr[2]);
		ImGui::Text("SPSR_abt: 0x%002X", cpu.spsr[3]);
		ImGui::Text("SPSR_und: 0x%002X", cpu.spsr[4]);
	}

	if (!arm_mode)
	{
		switch (user_mode)
		{
		case 1:
		{
			for (int n = 0; n < 15; n++)
			{
				ImGui::Text("Register %d: 0x%002X", n, cpu.system[n]);
			}
			break;
		}
		case 2:
		{
			for (int n = 0; n < 8; n++)
			{
				ImGui::Text("Register %d: 0x%002x", n, cpu.system[n]);
			}
			for (int n = 0; n < 6; n++)
			{
				ImGui::Text("Register %d: 0x%002x", n + 8, cpu.fiq[n]);
			}

			ImGui::Text("R13_fiq: 0x%002X", cpu.fiq[6]);
			ImGui::Text("R14_fiq: 0x%002X", cpu.fiq[7]);

			break;
		}
		case 3:
		{
			for (int n = 0; n < 13; n++)
				ImGui::Text("Register %d: 0x%002X", n, cpu.system[n]);

			ImGui::Text("R13_svc: 0x%002X", cpu.supervisor[0]);
			ImGui::Text("R14_svc: 0x%002X", cpu.supervisor[1]);

			break;
		}
		case 4:
		{
			for (int n = 0; n < 13; n++)
				ImGui::Text("Register %d: 0x%002X", n, cpu.system[n]);
			ImGui::Text("R13_abt: 0x%002X", cpu.abort[0]);
			ImGui::Text("R14_abt: 0x%002X", cpu.abort[1]);
			break;
		}
		case 5:
		{
			for (int n = 0; n < 13; n++)
				ImGui::Text("Register %d: 0x%002X", n, cpu.system[n]);
			ImGui::Text("R13_irq: 0x%002X", cpu.irq[0]);
			ImGui::Text("R14_irq: 0x%002X", cpu.irq[1]);
			break;
		}
		case 6:
		{
			for (int n = 0; n < 13; n++)
				ImGui::Text("Register %d: 0x%002X", n, cpu.undefined[n]);
			ImGui::Text("R13_und: 0x%002X", cpu.undefined[0]);
			ImGui::Text("R14_und: 0x%002X", cpu.undefined[1]);
			break;
		}
		}
	}
	else
	{
		switch (user_mode)
		{
		case 1:
		{
			for (int n = 0; n < 8; n++)
				ImGui::Text("Register %d: 0x%002X", n, cpu.system[n]);
			ImGui::Text("Register %d: 0x%002X", 13, cpu.system[13]);
			ImGui::Text("Register %d: 0x%002X", 14, cpu.system[14]);
			break;
		}
		case 2:
		{
			for (int n = 0; n < 8; n++)
				ImGui::Text("Register %d: 0x%002X", n, cpu.system[n]);
			ImGui::Text("SP_fiq: 0x%002X", cpu.fiq[6]);
			ImGui::Text("LR_fiq: 0x%002X", cpu.fiq[7]);
			break;
		}
		case 3:
		{
			for (int n = 0; n < 8; n++)
				ImGui::Text("Register %d: 0x%002X", n, cpu.system[n]);
			ImGui::Text("SP_svc: 0x%002X", cpu.supervisor[0]);
			ImGui::Text("LR_svc: 0x%002X", cpu.supervisor[1]);
			break;
		}
		case 4:
		{
			for (int n = 0; n < 8; n++)
				ImGui::Text("Register %d: 0x%002X", n, cpu.system[n]);
			ImGui::Text("SP_abt: 0x%002X", cpu.abort[0]);
			ImGui::Text("LR_abt: 0x%002X", cpu.abort[1]);
			break;
		}
		case 5:
		{
			for (int n = 0; n < 8; n++)
				ImGui::Text("Register %d: 0x%002X", n, cpu.system[n]);
			ImGui::Text("SP_irq: 0x%002X", cpu.irq[0]);
			ImGui::Text("LR_irq: 0x%002X", cpu.irq[1]);
			break;
		}
		case 6:
		{
			for (int n = 0; n < 8; n++)
				ImGui::Text("Register %d: 0x%002X", n, cpu.system[n]);
			ImGui::Text("SP_und: 0x%002X", cpu.undefined[0]);
			ImGui::Text("LR_und: 0x%002X", cpu.undefined[1]);
			break;
		}
		}
	}
	ImGui::Text("CPSR: 0x%002X", cpu.cpsr);
	ImGui::Text("Program Counter: 0x%002X", cpu.system[15]);
}

void Gui::ShowDisplay(Lcd& ppu)
{
	if (ImGui::Begin("Display")) {
		//const auto size = ImGui::GetWindowSize();
		//const auto scale_x = size.x / width;
		//const auto scale_y = size.y / height;
		//const auto scale = scale_x < scale_y ? scale_x : scale_y;

		//screen.update(ppu.buffer); // Present the buffer that's not being currently written to
		//sf::Sprite sprite(screen);
		//sprite.setScale(scale, scale);

		//ImGui::Image(sprite);
		//ImGui::End();
	}
}

void Gui::initImguiSfml()
{
	//init IMgui stuff
	window.setVerticalSyncEnabled(true);
	ImGui::SFML::Init(window);
	float color[3] = { 0.f, 0.f, 0.f };
	window.resetGLStates();
}

void Gui::imguiMainMenuBar()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("Load"))
		{
			ImGui::MenuItem("Load Rom", NULL, &load_rom);
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Emulator"))
		{
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Configure"))
		{
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Debugger"))
		{
			ImGui::MenuItem("Cpu Debugger", NULL, &cpu_debug_window);
			ImGui::MenuItem("Ppu Debugger", NULL, &ppu_debug_window);
			ImGui::MenuItem("Memory Viewer", NULL, &memory_window);
			ImGui::MenuItem("Save State", NULL, &save_state);
			ImGui::MenuItem("Load State", NULL, &load_state);
			ImGui::MenuItem("Display", NULL, &show_display);
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}

void Gui::MainWindow(Arm& cpu, Bus& bus, Lcd& ppu)
{
	window.create(sf::VideoMode(640, 480), "BlueAdvance");

	initImguiSfml();

	cpu.flushPipeline();

	cpu.resetCpu();

	while (window.isOpen()) {

		sf::Event event;
		while (window.pollEvent(event)) {
			ImGui::SFML::ProcessEvent(event);

			if (event.type == sf::Event::Closed) {
				window.close();
			}
		}

		ImGui::SFML::Update(window, deltaClock.restart());

		imguiMainMenuBar();

		if(cpu_debug_window) CpuDebugWindow(cpu, ppu, bus);

		if(ppu_debug_window) PpuDebugWindow(ppu);

		if (memory_window) MemoryWindow(bus);

		if (show_display) ShowDisplay(ppu);

		window.clear(bgColor); // fill background with color
		ImGui::SFML::Render(window);
		window.display();
	}

	ImGui::SFML::Shutdown();
}

void Gui::CpuDebugWindow(Arm& cpu, Lcd& ppu, Bus& bus)
{
	ImGui::Begin("CPU view");

	ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
	if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags))
	{
		if (ImGui::BeginTabItem("Cpu State"))
		{
			switch (cpu.cpsr & 0x1F)
			{
			case 0b10000: ImGui::Text("Current Mode : User"); break;
			case 0b10001: ImGui::Text("Current Mode : FIQ"); break;
			case 0b10010: ImGui::Text("Current Mode : IRQ"); break;
			case 0b10011: ImGui::Text("Current Mode : Supervisor"); break;
			case 0b10111: ImGui::Text("Current Mode : Abort"); break;
			case 0b11011: ImGui::Text("Current Mode : Undefined"); break;
			case 0b11111: ImGui::Text("Current Mode : sys"); break;
			};

			if(cpu.sta_res())
				ImGui::Text("Current State: Thumb");
			else
				ImGui::Text("Current State: Arm");

			ImGui::Text("Opcode: 0x%X", cpu.opcode);

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Register View"))
		{
			static const char* items[] = { "sys", "Fiq", "Supervisor", "Abort", "Irq", "Undefined", "Flags" };
			static int selected_item = 0;
			ImGui::Combo("Mode", &selected_item, items, IM_ARRAYSIZE(items), 7);
			user_mode = selected_item + 1;

			ImGui::SameLine();

			static bool checked = false;
			ImGui::Checkbox("Thumb", &checked);
			arm_mode = checked;

			ImGui::TextColored(ImVec4(1, 1, 0, 1), "Registers in mode:");

			ImGui::BeginChild("Scrolling");
			writeRegisters(cpu);
			ImGui::EndChild();


			static int i0 = 123;
			ImGui::InputInt("input X:", &i0);

			if (ImGui::Button("Step")) { 
				cpu.stepCpu(); 
				for(int i = 0; i < 4; i++)
					ppu.stepLcd(); 
				bus.HandleInterrupts();
				bus.DoDma();
				bus.DoTimers();
			}

			ImGui::SameLine();

			if (ImGui::Button("Step For X Cycles")) {
				int cycles = 0;
				while (cycles <= i0)
				{
					cpu.stepCpu();
					for (int i = 0; i < 4; i++)
						ppu.stepLcd();
					bus.HandleInterrupts();
					bus.DoDma();
					bus.DoTimers();
					cycles += 1;
				}
			}

			ImGui::SameLine();

			if (ImGui::Button("Break")) { 
				int cycles = 0;
				while (true) {
					cpu.stepCpu();
					for (int i = 0; i < 4; i++)
						ppu.stepLcd();
					bus.HandleInterrupts();
					bus.DoDma();
					bus.DoTimers();
					if (cpu.opcode == 0xe3a0c301)
						break;
				//	printf("CYCLES: %d SP_IRQ: 0x%X, PC: 0X%X\n", cycles, cpu.irq[0], cpu.system[15]);
					cycles += 1;
				} 

				//080004E8 - first write
			}

			ImGui::SameLine();

			if (ImGui::Button("Reset")) {
				for (int i = 0xE000; i < 0xE500; i += 16)
				{
					for (int j = 0; j < 16; j+=2)
					{
						u8 data = bus.VRAM[i + j];
						u8 data2 = bus.VRAM[i + j + 1];
						u16 final = data | (data2 << 8);
						printf("0x%002X\t", final);
					}
					printf("\n");
				}
			}

			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}

	ImGui::End(); // end window
}

void Gui::PpuDebugWindow(Lcd& ppu)
{
	ImGui::Begin("PPU view");

	ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
	if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags))
	{
		if (ImGui::BeginTabItem("Ppu State"))
		{

			ImGui::Text("Dot: %d", ppu.Cycles_Per_Line);

			ImGui::Text("Scanline: %d", ppu.LcdRead16(REG_VCOUNT));

			ppu.LcdRead16(REG_DISPSTAT) & 0x1 ? ImGui::Text("Vblank Set") : ImGui::Text("Vblank Unset");
			ppu.LcdRead16(REG_DISPSTAT) & 0x2 ? ImGui::Text("Hblank Set") : ImGui::Text("Hblank Unset");

			ppu.LcdRead16(REG_IME) & 0x1 ? ImGui::Text("IME: Set") : ImGui::Text("IME: Unset");

			ImGui::BeginChild("Scrolling");
			u16 reg_if_data = ppu.LcdRead16(REG_IF);
			ImGui::Text("REG_IF : 0x%X", reg_if_data);
			reg_if_data & 0x1 ? ImGui::Text("REG_IF : VBL Set") : ImGui::Text("REG_IF: VBL Unset");
			reg_if_data & 0x2 ? ImGui::Text("REG_IF : HBL Set") : ImGui::Text("REG_IF: HBL Unset");
			reg_if_data & 0x4 ? ImGui::Text("REG_IF : VCOUNT Set") : ImGui::Text("REG_IF: VCOUNT Unset");
			ImGui::EndChild();

			ImGui::BeginChild("Scrolling");
			u16 reg_ie_data = ppu.LcdRead16(REG_IE);
			ImGui::Text("REG_IE : 0x%X", reg_ie_data);
			reg_ie_data & 0x1 ? ImGui::Text("REG_IE : VBL Set") : ImGui::Text("REG_IE: VBL Unset");
			reg_ie_data & 0x2 ? ImGui::Text("REG_IE : HBL Set") : ImGui::Text("REG_IE: HBL Unset");
			reg_ie_data & 0x4 ? ImGui::Text("REG_IE : VCOUNT Set") : ImGui::Text("REG_IE: VCOUNT Unset");
			ImGui::EndChild();

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Irq"))
		{

			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	ImGui::End(); // end window
}

void Gui::MemoryWindow(Bus& bus)
{
	if (ImGui::CollapsingHeader("BIOS")) { mem_edit.DrawWindow("BIOS", bus.BIOS.mem.data(), bus.BIOS.mem.size()); }
	if (ImGui::CollapsingHeader("IWRAM")) { mem_edit.DrawWindow("IWRAM", bus.IWRAM.mem.data(), bus.IWRAM.mem.size()); }
	if (ImGui::CollapsingHeader("EWRAM")) { mem_edit.DrawWindow("EWRAM", bus.EWRAM.mem.data(), bus.EWRAM.mem.size()); }
	if (ImGui::CollapsingHeader("VRAM")) { mem_edit.DrawWindow("VRAM", bus.VRAM.mem.data(), bus.VRAM.mem.size()); }
	if (ImGui::CollapsingHeader("OAM")) { mem_edit.DrawWindow("OAM", bus.OAM.mem.data(), bus.OAM.mem.size()); }
	if (ImGui::CollapsingHeader("IOREG")) { mem_edit.DrawWindow("IO", bus.IOREG.mem.data(), bus.IOREG.mem.size()); }
	if (ImGui::CollapsingHeader("COLOR")) { mem_edit.DrawWindow("COLOR", bus.COLOR.mem.data(), bus.COLOR.mem.size()); }
	if (ImGui::CollapsingHeader("Game Pak region")) { mem_edit.DrawWindow("Game Pak region 1", bus.PAK1.mem.data(), bus.PAK1.mem.size()); }
}
