#include "bus.hpp"

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

	BIOS.load("C:\\Users\\Laxmi\\OneDrive\\Documents\\Projects\\gba\\external\\gba_bios.bin", 0x00, 16384);
	PAK1.load("C:\\Users\\Laxmi\\OneDrive\\Documents\\Projects\\gba\\external\\roms\\Mario Kart.gba", 0x0000000, 33554432);
    
	destAddr[0] = 0;
	destAddr[1] = 0;
	destAddr[2] = 0;
	destAddr[3] = 0;

	srcAddr[0] = 0;
	srcAddr[1] = 0;
	srcAddr[2] = 0;
	srcAddr[3] = 0;

	prevState[0] = false;
	prevState[1] = false;
	prevState[2] = false;
	prevState[3] = false;

	currState[0] = false;
	currState[1] = false;
	currState[2] = false;
	currState[3] = false;

	prevHblank.fill(false);
	currHblank.fill(false);
	prevVblank.fill(false);
	currVblank.fill(false);
}

void Bus::ConnectCPU(Arm* ptr)
{
	cpuPtr = ptr;
}

void Bus::ConnectPPU(Lcd* ptr)
{
	lcdPtr = ptr;
}

u32 Bus::BusRead16(u32 addr)
{
	u8 byte1 = BusRead(addr + 0);
	u8 byte2 = BusRead(addr + 1);
	u32 value = (byte2 << 8) | (byte1);
	return value;
}

u32 Bus::BusRead32(u32 addr)
{
	u8 byte1 = BusRead(addr + 0);
	u8 byte2 = BusRead(addr + 1);
	u8 byte3 = BusRead(addr + 2);
	u8 byte4 = BusRead(addr + 3);
	u32 value = (byte4 << 24) | (byte3 << 16) | (byte2 << 8) | (byte1);
	//if addr is less than 0x4000, then it's reading from bios
	// if(addr < 0x4000)
	// {
	// 	u32 temp = cpuPtr->ReadFromPC();
	// 	//if pc is within bios then no open bios, set latch value and return it
	// 	if(temp < 0x4000)
	// 	{
	// 		if((temp == 0xe3a02004) || (temp == 0xe25ef004) || (temp == 0xe55ec002))
	// 			last_value = value;
	// 	}
	// 	else
	// 	{
	// 		//however, it is now open bios, return last sucessfully fetched bios value
	// 		return last_value;
	// 	}
	// }
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

void Bus::write_mmio(u32 addr, u32 data, u8 mode)
{
	if (mode == 0)
	{
		if (addr == 0x40000BA) BusWrite16(addr, data & 0xFFFF);
		else if (addr == 0x40000C6) BusWrite16(addr, data & 0xFFFF);
		else if (addr == 0x40000D2) BusWrite16(addr, data & 0xFFFF);
		else if (addr == 0x40000DE) BusWrite16(addr, data & 0xFFFF);
		else if (addr == REG_VCOUNT) {}
		else if (addr == 0x4000100) {
			tmrReload[0] = data & 0xFFFF;
			BusWrite16(addr + 2, (data >> 16) & 0xFFFF);
		}
		else if (addr == 0x4000102) BusWrite16(addr, data & 0xFFFF);
		else if (addr == 0x4000104) {
			tmrReload[1] = data & 0xFFFF;
			BusWrite16(addr + 2, (data >> 16) & 0xFFFF);
		}
		else if (addr == 0x4000106) BusWrite16(addr, data & 0xFFFF);
		else if (addr == 0x4000108) {
			tmrReload[2] = data & 0xFFFF;
			BusWrite16(addr + 2, (data >> 16) & 0xFFFF);
		}
		else if (addr == 0x400010A) BusWrite16(addr, data & 0xFFFF);
		else if (addr == 0x400010C) {
			tmrReload[3] = data & 0xFFFF;
			BusWrite16(addr + 2, (data >> 16) & 0xFFFF);
		}
		else if (addr == 0x400010E) BusWrite16(addr, data & 0xFFFF);
		//else if ((addr == REG_BG2X) && !inVblank) lcdPtr->BG2aff.ref_x = data;
		//else if ((addr == REG_BG2Y) && !inVblank) lcdPtr->BG2aff.ref_y = data;
		//else if ((addr == REG_BG3X) && !inVblank) lcdPtr->BG3aff.ref_x = data;
		//else if ((addr == REG_BG3Y) && !inVblank) lcdPtr->BG3aff.ref_y = data;
		else BusWrite32(addr, data);
	}
	else if (mode == 1)
	{
		if (addr == REG_VCOUNT) {}
		else if (addr == 0x4000100) {
			tmrReload[0] = data & 0xFFFF;
		}
		else if (addr == 0x4000104) {
			tmrReload[1] = data & 0xFFFF;
		}
		else if (addr == 0x4000108) {
			tmrReload[2] = data & 0xFFFF;
		}
		else if (addr == 0x400010c) {
			tmrReload[3] = data & 0xFFFF;
		}

		else BusWrite16(addr, data & 0xFFFF);
	}
	else if (mode == 2)
	{
		if (addr == 0x4000100) {
			tmrReload[0] &= 0xFF00;
			tmrReload[0] |= data & 0xFF;
		}
		else if (addr == 0x4000101) {
			tmrReload[0] &= 0x00FF;
			tmrReload[0] |= ((data & 0xFF) << 8);
		}
		else if (addr == 0x4000104) {
			tmrReload[1] &= 0xFF00;
			tmrReload[1] |= data & 0xFF;
		}
		else if (addr == 0x4000105) {
			tmrReload[1] &= 0x00FF;
			tmrReload[1] |= ((data & 0xFF) << 8);
		}
		else if (addr == 0x4000108) {
			tmrReload[2] &= 0xFF00;
			tmrReload[2] |= data & 0xFF;
		}
		else if (addr == 0x4000109) {
			tmrReload[2] &= 0x00FF;
			tmrReload[2] |= ((data & 0xFF) << 8);
		}
		else if (addr == 0x400010c) {
			tmrReload[3] &= 0xFF00;
			tmrReload[3] |= data & 0xFF;
		}
		else if (addr == 0x400010d) {
			tmrReload[3] &= 0x00FF;
			tmrReload[3] |= ((data & 0xFF) << 8);
		}
		else BusWrite(addr, data & 0xFF);
	}

	if (!inVblank)
	{
		switch (addr)
		{
		case REG_BG2X:
		case REG_BG2X + 1:
		case REG_BG2X + 2:
		case REG_BG2X + 3:
			lcdPtr->BG2aff.ref_x = BusRead32(REG_BG2X);
		}

		switch (addr)
		{
		case REG_BG2Y:
		case REG_BG2Y + 1:
		case REG_BG2Y + 2:
		case REG_BG2Y + 3:
			lcdPtr->BG2aff.ref_y = BusRead32(REG_BG2Y);
		}

		switch (addr)
		{
		case REG_BG3X:
		case REG_BG3X + 1:
		case REG_BG3X + 2:
		case REG_BG3X + 3:
			lcdPtr->BG3aff.ref_x = BusRead32(REG_BG3X);
		}

		switch (addr)
		{
		case REG_BG3Y:
		case REG_BG3Y + 1:
		case REG_BG3Y + 2:
		case REG_BG3Y + 3:
			lcdPtr->BG3aff.ref_y = BusRead32(REG_BG3Y);
		}
	}
}

void Bus::GetDmaData()
{
	for (int i = 0; i < 4; i++)
	{
		u16 dma_data = BusRead16(0x40000BA + 0xC * i);

		DmaChannels[i].dma_dest_adjust = (dma_data >> 5) & 0x3;
		DmaChannels[i].dma_src_adjust = (dma_data >> 7) & 0x3;
		DmaChannels[i].dma_repeat = (dma_data >> 9) & 0x1;
		DmaChannels[i].dma_cs = (dma_data >> 10) & 0x1;
		DmaChannels[i].dma_tm = (dma_data >> 12) & 0x3;
		DmaChannels[i].dma_int = (dma_data >> 14) & 0x1;
		DmaChannels[i].dma_en = (dma_data >> 15) & 0x1;

		prevState[i] = currState[i];
		currState[i] = DmaChannels[i].dma_en;

		prevHblank[i] = currHblank[i];
		currHblank[i] = inHblank && !inVblank;

		prevVblank[i] = currVblank[i];
		currVblank[i] = inVblank;
	}
}

void Bus::DoDmaCalculations(int channel)
{
	bool complete = false;
	if ((prevState[channel] == false) && (currState[channel] == true))
	{
		//Reload SAD, DAD and CNT_L
		srcAddr[channel] = BusRead32(0x40000B0 + 0xC * channel) & (channel == 0 ? 0x7FFFFFF : 0xFFFFFFF);
		destAddr[channel] = BusRead32(0x40000B4 + 0xC * channel) & (channel == 3 ? 0xFFFFFFF : 0x7FFFFFF);
		transferNum[channel] = BusRead16(0x40000B8 + 0xC * channel) & (channel == 3 ? 0xFFFF : 0x3FFF);
	}

	u32 number_of_transfers = transferNum[channel];
	u32 dad_adjust = DmaChannels[channel].dma_dest_adjust;
	u32 dma_mode = DmaChannels[channel].dma_cs;

	u16 scanline = BusRead16(REG_VCOUNT);

	switch (DmaChannels[channel].dma_tm)
	{
	case 0: doTransfer = true; break;
	case 1: doTransfer = (prevVblank[channel] == false) && (currVblank[channel] == true); break;
	case 2: doTransfer = (prevHblank[channel] == false) && (currHblank[channel] == true); break;
	case 3:
		if ((channel == 1) || (channel == 2))
		{
			//FIFO mode, some controls bits are ignored
			number_of_transfers = 4;
			dma_mode = 1;
			dad_adjust = 2;
			doTransfer = false;
		}
		else if (channel == 3)
		{
			//video capture mode
			//doTransfer = (BusRead16(REG_VCOUNT) >= 2) && (BusRead16(REG_VCOUNT) <= 162) && (prevScanline != BusRead16(REG_VCOUNT));
			//if (prevScanline != BusRead16(REG_VCOUNT))
			//{
			//	prevScanline = BusRead16(REG_VCOUNT);
			//}
			printf("Using video capture mode!");
		}
		break;
	}

	//a value of 0 means max length dma
	if (number_of_transfers == 0)
	{
		if (channel != 3)
		{
			number_of_transfers = 0x4000;
		}
		else
		{
			number_of_transfers = 0x10000;
		}
	}

	if (doTransfer)
	{
		//	printf("DMA %d enabled : Destination : %X Source: %X Control : %X Number of Transfers : %X\n", channel, destAddr[channel], srcAddr[channel], BusRead16(0x40000BA + 0xC * channel), transferNum[channel]);
		for (int i = 0; i < number_of_transfers; i++)
		{
			if (dma_mode)
			{
				BusWrite32(destAddr[channel] & ~3, BusRead32(srcAddr[channel] & ~3));
			}
			else
			{
				BusWrite16(destAddr[channel] & ~1, BusRead16(srcAddr[channel] & ~1));
			}

			if (dad_adjust == 0) destAddr[channel] += DmaChannels[channel].dma_cs ? 4 : 2;
			else if (dad_adjust == 1) destAddr[channel] -= DmaChannels[channel].dma_cs ? 4 : 2;
			else if (dad_adjust == 3) destAddr[channel] += DmaChannels[channel].dma_cs ? 4 : 2;

			if (DmaChannels[channel].dma_src_adjust == 0) srcAddr[channel] += DmaChannels[channel].dma_cs ? 4 : 2;
			else if (DmaChannels[channel].dma_src_adjust == 1) srcAddr[channel] -= DmaChannels[channel].dma_cs ? 4 : 2;
		}
		complete = true;
	}

	if (complete)
	{
		//	printf("DMA TRIGGERED, CLEARING DATA!");
		if (!DmaChannels[channel].dma_repeat)
		{
			//Clear enable bit in register
			//currState[channel] = false;
			BusWrite32(0x40000B8 + 0xC * channel, BusRead32(0x40000B8 + 0xC * channel) & ~0x80000000);
		}
		else
		{
			transferNum[channel] = BusRead16(0x40000B8 + 0xC * channel);
			if (dad_adjust == 3)
			{
				destAddr[channel] = BusRead32(0x40000B4 + 0xC * channel);
			}
		}

		if (DmaChannels[channel].dma_int)
		{
			doDmaInterrupt(channel + 1); //because channel is from [0, 3] and you want [1, 4]
		}
	}

}

void Bus::DoDma()
{
	GetDmaData();

	if (DmaChannels[0].dma_en)
	{
		DoDmaCalculations(0);
	}

	if (DmaChannels[1].dma_en)
	{
		DoDmaCalculations(1);
	}

	if (DmaChannels[2].dma_en)
	{
		DoDmaCalculations(2);
	}

	if (DmaChannels[3].dma_en)
	{
		DoDmaCalculations(3);
	}
}

void Bus::doDmaInterrupt(int channel)
{
	IOREG[(REG_IF + 1) - 0x04000000] |= channel;
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

		if ((addr == REG_IF) || (addr == (REG_IF + 1)))
		{
			u8 value = lcdPtr->LcdRead16(REG_IF) >> ((addr - REG_IF) * 8);
			data = value & ~data;
		}

		if (addr == 0x4000301)
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

u8 Bus::BusRead(uint32_t addr)
{
	u8 range = (addr >> 24) & 0xF;
	switch (range)
	{
	case 0x00:
	case 0x01:
		return BIOS[addr % 0x4000];
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

	if (IE & IF)
	{
		IS_THE_CPU_IN_HALT = false;
		//IRQ Generated
		if (IME && !isCpsrBit7set)
		{
			cpuPtr->InterruptRequest();
		}
	}
}

void Bus::write_controls(uint8_t* keys)
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
	int frameTime = 0, frameTotal = 0, fps = 0;
	u32 frameStart = 0;
	int frameNumber = 0;
	cpuPtr->flushPipeline();

	while (running)
	{
		frameStart = SDL_GetTicks();

		uint8_t* kb = (uint8_t*)SDL_GetKeyboardState(NULL);
		write_controls(kb);
		int cycles = CYCLES_PER_FRAME;
		while (cycles > 0)
		{
			if (!IS_THE_CPU_IN_HALT)
				cpuPtr->stepCpu();
			lcdPtr->stepLcd();
			DoTimers();
			DoDma();
			HandleInterrupts();
			cycles -= 1;
		}
		lcdPtr->DrawFrame();
		lcdPtr->ClearBuffer();
		while (SDL_PollEvent(&event)) {
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

		std::string win_title = " - ";
		std::string fps_string = std::to_string(fps);
		win_title += fps_string;

		SDL_SetWindowTitle(lcdPtr->window, win_title.c_str());
	}
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
