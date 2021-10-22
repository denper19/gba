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

	bus.ConnectCPU(&cpu);
	bus.ConnectPPU(&ppu);
	//bus.ConnectDMA(&dma);
	//bus.ConnectTMR(&tmr);

	cpu.ConnectBus(&bus);
	//dma.ConnectBus(&bus);
	//tmr.ConnectBus(&bus);
	ppu.ConnectBusToPPU(&bus);
	//gui.MainWindow(cpu, bus, ppu);
	bus.Run();
	return 0;
}
