#pragma once

/*

The purpose of this class is to handle memory reads/writes, interrupts and to transfer data 
between components. In other words it behaves as the bus(hence the name) in computers used for
said purpose

*/

#include <string>
#include "mem.hpp"
#include "cpu.hpp"
#include "ppu.hpp"
#include "dma.hpp"
#include "tmr.hpp"
#include <atomic>

#define KB 1024
#define CYCLES_PER_FRAME (240 + 68) * (160 + 68) * 4

class Bus
{
private:

	int fps{ 0 };

	bool HALT_CPU = false;
	bool IS_THE_CPU_IN_HALT = false;
	bool irq_prevState = false;
	bool irq_currState = false;

	Arm* cpuPtr;
	Lcd* lcdPtr;
	Dma* dmaPtr;
	Tmr* tmrPtr;

	uint8_t* keys{ (uint8_t*)SDL_GetKeyboardState(NULL) };

	friend class GuiInterface;

public:

	bool paused = false;
	bool c = false;

	u32 latch = 0;
	u32 old_latch = 0;

	bool inHblank, inVblank;

	Mem BIOS, IWRAM, EWRAM, IOREG, COLOR,
		VRAM, OAM, PAK1, PAK2, PAK3, SRAM;

	Bus();

	void ConnectCPU(Arm*);
	void ConnectPPU(Lcd*);
	void ConnectDMA(Dma*);
	void ConnectTMR(Tmr*);

	u8   BusRead(u32);
	void write_mmio(u32 addr, u32 data, u8 mode);
	u32	 BusRead16(u32);
	u32	 BusRead32(u32);
	void BusWrite(u32, u8);
	void BusWrite16(u32, u16);
	void BusWrite32(u32, u32);

	void setVblank();
	void setHblank();
	void clrVblank();
	void clrHblank();
	void HandleInterrupts();
	void write_controls();

	void Run();
	void Step();

	s32 getInternalX();
	s32 getInternalY();

	~Bus();
};

