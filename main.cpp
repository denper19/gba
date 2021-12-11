#include "bus.hpp"
#include "cpu.hpp"
#include "ppu.hpp"


int main()
{
	Bus bus;
	Arm cpu;
	Lcd ppu;
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

	cpu.ConnectBus(&bus);
	ppu.ConnectBusToPPU(&bus);

	bus.Run();

	return 0;
}
