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

	bus.ConnectCPU(&cpu);
	bus.ConnectPPU(&ppu);
	bus.ConnectDMA(&dma);
	bus.ConnectTMR(&tmr);

	cpu.ConnectBus(&bus);
	ppu.ConnectBus(&bus);
	dma.ConnectBus(&bus);
	tmr.ConnectBus(&bus);

	//cpu.SkipBios();
	bus.Run();

	return 0;
}
