#include "bus.hpp"
#include "gui.hpp"

u32 table[] = {0xffffff00, 0xffff00ff, 0xff00ffff, 0x00ffffff};

Bus::Bus()
{
	BIOS.resize(16 * KB);
	IWRAM.resize(256 * KB);
	EWRAM.resize(32 * KB);
	COLOR.resize(1 * KB);
	VRAM.resize(96 * KB);
	OAM.resize(1 * KB);
	IOREG.resize(0x400);
	PAK1.resize(33554432);
	PAK2.resize(0x02000000);
	PAK3.resize(0x02000000);
	SRAM.resize(64 * KB);

	BIOS.init(0x00);
	IWRAM.init(0x00);
	EWRAM.init(0x00);
	IOREG.init(0x00);
	COLOR.init(0x00);
	VRAM.init(0x00);
	PAK1.init(0x00);
	PAK2.init(0x00);
	PAK3.init(0x00);
	SRAM.init(0x00);

	cpuPtr = nullptr;
	lcdPtr = nullptr;
	dmaPtr = nullptr;
	tmrPtr = nullptr;

	BIOS.load("C:\\Users\\Laxmi\\OneDrive\\Documents\\Projects\\gba\\external\\gba_bios.bin", 0x00, 16384);
	PAK1.load("C:\\Users\\Laxmi\\OneDrive\\Desktop\\file\\roms\\Mario Kart Super Circuit (U) [!].gba", 0x0000000, 33554432);
}

void Bus::ConnectCPU(Arm* ptr)
{
	cpuPtr = ptr;
}

void Bus::ConnectPPU(Lcd* ptr)
{
	lcdPtr = ptr;
}

void Bus::ConnectDMA(Dma* temp)
{
	dmaPtr = temp;
}

void Bus::ConnectTMR(Tmr* temp)
{
	tmrPtr = temp;
}

u32 Bus::BusRead16(u32 addr)
{
	u8 byte1 = BusRead(addr + 0);
	u8 byte2 = BusRead(addr + 1);
	u32 value = (byte2 << 8) | (byte1);
	if ((addr < 0x4000) && (cpuPtr->ReadFromPC() > 0x3FFF))
	{
		return printf("Beeeep 16\n");
	}
	return value;
}

u32 Bus::BusRead32(u32 addr)
{
	u8 byte1 = BusRead(addr + 0);
	u8 byte2 = BusRead(addr + 1);
	u8 byte3 = BusRead(addr + 2);
	u8 byte4 = BusRead(addr + 3);
	u32 value = (byte4 << 24) | (byte3 << 16) | (byte2 << 8) | (byte1);
	//if ((addr < 0x4000) && (cpuPtr->ReadFromPC() < 0x4000))
	//{
	//	//bios region, latch last succesfully fetched opcode
	//	latch = value;
	//}
	if ((addr < 0x4000) && (cpuPtr->ReadFromPC() > 0x3FFF))
	{
		return printf("Beeeep 32\n");
	}
	return value;
}

void Bus::BusWrite16(u32 addr, u16 data)
{
	u8 byte1 = (data >> 0) & 0xFF;
	u8 byte2 = (data >> 8) & 0xFF;
	BusWrite(addr + 0, byte1);
	BusWrite(addr + 1, byte2);
}

void Bus::BusWrite32(u32 addr, u32 data)
{
	u8 byte1 = (data >> 0) & 0xFF;
	u8 byte2 = (data >> 8) & 0xFF;
	u8 byte3 = (data >> 16) & 0xFF;
	u8 byte4 = (data >> 24) & 0xFF;
	BusWrite(addr + 0, byte1);
	BusWrite(addr + 1, byte2);
	BusWrite(addr + 2, byte3);
	BusWrite(addr + 3, byte4);
}

void Bus::BusWrite(u32 addr, u8 data)
{
	u8 range = (addr >> 24) & 0xF;

	switch (range)
	{
	case 0x02:
		IWRAM[(addr - 0x2000000) % 0x40000] = data;
		break;
	case 0x03:
		EWRAM[(addr - 0x03000000) % 0x8000] = data;
		break;
	case 0x04:

		if (!inVblank)
		{
			if ((addr == REG_BG2X_H) || (addr == (REG_BG2X_H + 1)))
			{
				if (addr == REG_BG2X_H)
				{
					lcdPtr->BG2aff.ref_x &= 0xFF00FFFF;
					lcdPtr->BG2aff.ref_x |= (data << 16);
				}
				else
				{
					lcdPtr->BG2aff.ref_x &= 0x00FFFFFF;
					lcdPtr->BG2aff.ref_x |= (data << 24);
					lcdPtr->BG2aff.ref_x <<= 4;
					lcdPtr->BG2aff.ref_x >>= 4;
				}
			}

			if ((addr == REG_BG2X_L) || (addr == (REG_BG2X_L + 1)))
			{
				if (addr == REG_BG2X_L)
				{
					lcdPtr->BG2aff.ref_x &= 0xFFFFFF00;
					lcdPtr->BG2aff.ref_x |= data;
				}
				else
				{
					lcdPtr->BG2aff.ref_x &= 0xFFFF00FF;
					lcdPtr->BG2aff.ref_x |= (data << 8);
				}
			}

			if ((addr == REG_BG2Y_H) || (addr == (REG_BG2Y_H + 1)))
			{
				if (addr == REG_BG2Y_H)
				{
					lcdPtr->BG2aff.ref_y &= 0xFF00FFFF;
					lcdPtr->BG2aff.ref_y |= (data << 16);
				}
				else
				{
					lcdPtr->BG2aff.ref_y &= 0x00FFFFFF;
					lcdPtr->BG2aff.ref_y |= (data << 24);
					lcdPtr->BG2aff.ref_y <<= 4;
					lcdPtr->BG2aff.ref_y >>= 4;
				}
			}

			if ((addr == REG_BG2Y_L) || (addr == (REG_BG2Y_L + 1)))
			{
				if (addr == REG_BG2Y_L)
				{
					lcdPtr->BG2aff.ref_y &= 0xFFFFFF00;
					lcdPtr->BG2aff.ref_y |= data;
				}
				else
				{
					lcdPtr->BG2aff.ref_y &= 0xFFFF00FF;
					lcdPtr->BG2aff.ref_y |= (data << 8);
				}
			}
		}

		if ((addr == REG_IF) || (addr == (REG_IF + 1)))
		{
			u8 value = lcdPtr->LcdRead16(REG_IF) >> ((addr - REG_IF) * 8);
			data = value & ~data;
		}

		if ((addr == 0x4000301))
			IS_THE_CPU_IN_HALT = true;

		if (addr < 0x4000400) IOREG[addr - 0x04000000] = data;

		break;

	case 0x05:
		COLOR[(addr - 0x05000000) % 0x400] = data;
		break;

	case 0x06:

		//VRAM is 96k(64k + 32k) repeated in 128k steps(64k + 32k + 32k) where the 32 k regions are mirrors of each other
		//Mirrors the address to 128 K range

		addr = (addr - 0x06000000) % 0x20000;

		if (addr >= 0x10000)
		{
			addr = ((addr - 0x10000) % 0x8000) + 0x10000;
			VRAM[addr] = data;
		}
		else VRAM[addr] = data;

		break;

	case 0x07:
		OAM[(addr - 0x07000000) % 0x400] = data;
		break;
	case 0x0e: case 0x0f:
		SRAM[(addr - 0x0E000000) % 0x10000] = data;
		break;
	}
}

u8 Bus::BusRead(u32 addr)
{
	u8 range = (addr >> 24) & 0xF;
	switch (range)
	{
	case 0x00:
	case 0x01:
	{
		if ((addr < 0x4000) && (cpuPtr->ReadFromPC() > 0x3FFF))
		{
			//rotate data based on addr
			u8 temp = latch >> ((addr & 3) * 8);
			return temp;
		}
		else if ((addr < 0x4000) && (cpuPtr->ReadFromPC() <= 0x3FFF))
		{
			//latch data and store byte depending on address
			return BIOS[addr];
			//if (cpuPtr->piping)
			//{
			//	latch &= table[addr & 3];
			//	latch |= (data << ((addr & 3) * 8));
			//}
			/*if(latch == 0xe55ec002)
				printf("Data : 0x%002x Latch : 0x%002x\n", data, latch);*/
		}
	}
	case 0x02:
		return IWRAM[(addr - 0x2000000) % 0x40000];
	case 0x03:
		return EWRAM[(addr - 0x03000000) % 0x8000];
	case 0x04:
		if (addr < 0x4000400) return IOREG[addr - 0x04000000];
		else return 0;
	case 0x05:
		return COLOR[(addr - 0x05000000) % 0x400];
	case 0x06:

		//VRAM is 96k(64k + 32k) repeated in 128k steps(64k + 32k + 32k) where the 32 k regions are mirrors of each other
		//Mirrors the address to 128 K range

		addr = (addr - 0x06000000) % 0x20000;

		if (addr >= 0x10000)
		{
			addr = ((addr - 0x10000) % 0x8000) + 0x10000;
			return VRAM[addr];
		}
		else return VRAM[addr];

	case 0x07:
		return OAM[(addr - 0x07000000) % 0x400];
	case 0x08: case 0x09:
	case 0x0a: case 0x0b:
	case 0x0c: case 0x0d:
		return PAK1[(addr - 0x08000000) % 0x02000000];
	case 0x0e:
	case 0x0f:
		return SRAM[(addr - 0x0E000000) % 0x10000];
	default:
		return 0;
	};
}

void Bus::HandleInterrupts()
{
	u16 IE = lcdPtr->LcdRead16(REG_IE);
	u16 IF = lcdPtr->LcdRead16(REG_IF);
	u16 IME = lcdPtr->LcdRead16(REG_IME) & 1;
	u16 isCpsrBit7set = cpuPtr->irq_res();

	irq_prevState = irq_currState;
	irq_currState = false;
	if (IE & IF)
	{
		IS_THE_CPU_IN_HALT = false;
		//IRQ Generated
		if (IME && !isCpsrBit7set)
		{
			cpuPtr->InterruptRequest();
			irq_currState = true;
		}
	}
}

void Bus::write_controls()
{
	u32 controller = 0x00;

	!keys[SDL_SCANCODE_X] ? controller |= 0x001 : controller &= ~0x001;//A
	!keys[SDL_SCANCODE_Z] ? controller |= 0x002 : controller &= ~0x002;//B
	!keys[SDL_SCANCODE_N] ? controller |= 0x004 : controller &= ~0x004;//START
	!keys[SDL_SCANCODE_M] ? controller |= 0x008 : controller &= ~0x008;//SELECT
	!keys[SDL_SCANCODE_D] ? controller |= 0x010 : controller &= ~0x010;//RIGHT
	!keys[SDL_SCANCODE_A] ? controller |= 0x020 : controller &= ~0x020;//LEFT
	!keys[SDL_SCANCODE_S] ? controller |= 0x040 : controller &= ~0x040;//DOWN
	!keys[SDL_SCANCODE_W] ? controller |= 0x080 : controller &= ~0x080;//UP
	!keys[SDL_SCANCODE_P] ? controller |= 0x100 : controller &= ~0x100;//R
	!keys[SDL_SCANCODE_L] ? controller |= 0x200 : controller &= ~0x200;//L

	cpuPtr->CpuWrite32(0x04000130, controller);
}

void Bus::Run()
{
	SDL_Event event;
	bool running = true;
	int frameTime = 0, frameTotal = 0;
	u32 frameStart = 0;
	int frameNumber = 0;

	cpuPtr->flushPipeline();

	while (!paused)
	{
		if (running)
		{
			frameStart = SDL_GetTicks();
			//runs emulator for one frame
			Step();

			lcdPtr->DrawFrame();
			//lcdPtr->ClearBuffer();

			//after each frame, poll keys
			while (SDL_PollEvent(&event))
			{
				if (event.type == SDL_QUIT) {
					running = false;
				}
			}

			frameTime = SDL_GetTicks() - frameStart;
			frameTotal += frameTime;

			if (frameTotal > 1000)
			{
				frameTotal -= 1000;
				fps = frameNumber;
				frameNumber = 0;
			}

			frameNumber++;

			std::string d = std::to_string(fps);
			SDL_SetWindowTitle(lcdPtr->window, d.c_str());
		}
	}
}

void Bus::Step()
{
	write_controls();
	int cycles = CYCLES_PER_FRAME;

	//run for one frame
	while (cycles > 0)
	{
		if (!IS_THE_CPU_IN_HALT)
			cpuPtr->stepCpu();
		lcdPtr->stepLcd();
		tmrPtr->DoTimers();
		dmaPtr->DoDma();
		HandleInterrupts();
		cycles -= 1;
	}
}

s32 Bus::getInternalX()
{
	return lcdPtr->getBG2X();
}

s32 Bus::getInternalY()
{
	return lcdPtr->getBG2Y();
}

void Bus::setVblank()
{
	BusWrite16(REG_DISPSTAT, BusRead16(REG_DISPSTAT) | 0x1);
	if (BusRead16(REG_DISPSTAT) & 0x8)
		IOREG[REG_IF - 0x04000000] |= 1;
}

void Bus::clrVblank()
{
	BusWrite16(REG_DISPSTAT, BusRead16(REG_DISPSTAT) & ~1);
}

void Bus::setHblank()
{
	BusWrite16(REG_DISPSTAT, BusRead16(REG_DISPSTAT) | 0x2);
	if (BusRead16(REG_DISPSTAT) & 0x10)
		IOREG[REG_IF - 0x04000000] |= 2;
}

void Bus::clrHblank()
{
	BusWrite16(REG_DISPSTAT, BusRead16(REG_DISPSTAT) & ~2);
}

Bus::~Bus()
{
	cpuPtr = NULL;
}
