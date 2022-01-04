#include "bus.hpp"
#include "cpu.hpp"
#include "ppu.hpp"
#include "dma.hpp"
#include "tmr.hpp"
#include "gui.hpp"

#include <thread>

int main()
{
	Bus bus;
	Arm cpu;
	Lcd ppu;
	Dma dma;
	Tmr tmr;
	GuiInterface gui;

	bus.ConnectCPU(&cpu);
	bus.ConnectPPU(&ppu);
	bus.ConnectDMA(&dma);
	bus.ConnectTMR(&tmr);

	cpu.ConnectBus(&bus);
	ppu.ConnectBus(&bus);
	dma.ConnectBus(&bus);
	tmr.ConnectBus(&bus);

	//cpu.SkipBios();

	std::thread t1(&Bus::Run, bus);
	gui.GuiMain(&ppu, &bus);
	t1.join();

	return 0;
}
