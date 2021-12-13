#include "bus.hpp"
#include "cpu.hpp"
#include "ppu.hpp"
#include "dma.hpp"
#include "tmr.hpp"

int main()
{
	Bus bus;
	Arm cpu;
	Lcd ppu;
	Dma dma;
	Tmr tmr;
//	Dma dma;
//	Tmr tmr;
//	Gui gui;
	//bus.ConnectDMA(&dma);
	//bus.ConnectTMR(&tmr);
	//dma.ConnectBus(&bus);
	//tmr.ConnectBus(&bus);
	//gui.MainWindow(cpu, bus, ppu);

	bus.ConnectCPU(&cpu);
	bus.ConnectPPU(&ppu);
	bus.ConnectDMA(&dma);
	bus.ConnectTMR(&tmr);

	cpu.ConnectBus(&bus);
	ppu.ConnectBus(&bus);
	dma.ConnectBus(&bus);
	tmr.ConnectBus(&bus);

	bus.Run();

	return 0;
}
