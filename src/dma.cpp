#include "dma.hpp"
#include "bus.hpp"

Dma::Dma()
{
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

void Dma::ConnectBus(Bus* temp)
{
	busPtr = temp;
}

void Dma::GetDmaData()
{
	for (int i = 0; i < 4; i++)
	{
		u16 dma_data = busPtr->BusRead16(0x40000BA + 0xC * i);

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
		currHblank[i] = busPtr->inHblank && !busPtr->inVblank;

		prevVblank[i] = currVblank[i];
		currVblank[i] = busPtr->inVblank;
	}
}

void Dma::DoDmaCalculations(int channel)
{
	bool complete = false;
	if ((prevState[channel] == false) && (currState[channel] == true))
	{
		//Reload SAD, DAD and CNT_L
		srcAddr[channel] = busPtr->BusRead32(0x40000B0 + 0xC * channel) & (channel == 0 ? 0x7FFFFFF : 0xFFFFFFF);
		destAddr[channel] = busPtr->BusRead32(0x40000B4 + 0xC * channel) & (channel == 3 ? 0xFFFFFFF : 0x7FFFFFF);
		transferNum[channel] = busPtr->BusRead16(0x40000B8 + 0xC * channel) & (channel == 3 ? 0xFFFF : 0x3FFF);
	}

	u32 number_of_transfers = transferNum[channel];
	u32 dad_adjust = DmaChannels[channel].dma_dest_adjust;
	u32 dma_mode = DmaChannels[channel].dma_cs;

	u16 scanline = busPtr->BusRead16(REG_VCOUNT);

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
				busPtr->BusWrite32(destAddr[channel] & ~3, busPtr->BusRead32(srcAddr[channel] & ~3));
			}
			else
			{
				busPtr->BusWrite16(destAddr[channel] & ~1, busPtr->BusRead16(srcAddr[channel] & ~1));
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
			busPtr->BusWrite32(0x40000B8 + 0xC * channel, busPtr->BusRead32(0x40000B8 + 0xC * channel) & ~0x80000000);
		}
		else
		{
			transferNum[channel] = busPtr->BusRead16(0x40000B8 + 0xC * channel);
			if (dad_adjust == 3)
			{
				destAddr[channel] = busPtr->BusRead32(0x40000B4 + 0xC * channel);
			}
		}

		if (DmaChannels[channel].dma_int)
		{
			doDmaInterrupt(channel + 1); //because channel is from [0, 3] and you want [1, 4]
		}
	}

}

void Dma::DoDma()
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

void Dma::doDmaInterrupt(int channel)
{
	busPtr->IOREG[(REG_IF + 1) - 0x04000000] |= channel;
}
