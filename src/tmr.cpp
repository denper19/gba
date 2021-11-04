#include "tmr.hpp"

Tmr::Tmr()
{   
    timerLUT[0] = 1;
	timerLUT[1] = 64;
	timerLUT[2] = 256;
	timerLUT[3] = 1024;
}

void Tmr::ConnectBus(Bus* temp) 
{
    bus_ptr = temp;
}

void Tmr::stepTimers()
{
	for (int i = 0; i < 4; i++)
	{
		u32 tmrCntReg = 0x4000102 + 0x4 * i;
		u32 tmrDatReg = 0x4000100 + 0x4 * i;
		u16 tmrCntVal = BusRead16(tmrCntReg);
		//get timer enable bit and restage the state pipeline
		prevTmrState[i] = currTmrState[i];
		currTmrState[i] = (tmrCntVal >> 7) & 0x1;
		//check for rising edge 
		if ((prevTmrState[i] == false) && (currTmrState[i] == true))
		{
			//Rising edge detected, which means reload data value
			BusWrite16(tmrDatReg, tmrReload[i]);
		}
		if (currTmrState[i])
		{

			u32 tmrDatVal = BusRead16(tmrDatReg);
			u8 tmrFreq = tmrCntVal & 0x3;
			u8 tmrCascade = (tmrCntVal >> 2) & 0x1;
			u8 tmrIrq = (tmrCntReg >> 6) & 0x1;

			bool IncrementTmr = false;

			if (tmrCascade)
			{
				//Counting is done if the prev timer overflows, not possible for timer 0
				if (i != 0)
				{
					//Check if prev tmr overflows
					u32 prevTimer = BusRead16(0x4000100 + 0x4 * (i - 1));
					if (tmrOverflow[i - 1])
					{
						//timer has overflowed so increment
						IncrementTmr = true;
						//tmrOverflow[i - 1] = false;
					}
				}
			}
			else
			{
				u32 timerTicks = timerLUT[tmrFreq];
				if (tmrCounter[i] >= timerTicks)
				{
					tmrCounter[i] -= timerTicks;
					IncrementTmr = true;
				}
			}

			tmrOverflow[i] = false;

			if (IncrementTmr)
			{
				if (tmrDatVal == 0xFFFF)
				{
					tmrOverflow[i] = true;
					BusWrite16(tmrDatReg, tmrReload[i]);
					if (tmrIrq) tmrInterrupt(i);
				}
				else
				{
					BusWrite16(tmrDatReg, tmrDatVal + 1);
				}
			}

			tmrCounter[i]++;
		}
	}
}

void Tmr::tmrInterrupt(int timer)
{
	// switch (timer)
	// {
	// case 0:
	// 	IOREG[(REG_IF + 0) - 0x4000000] |= 0x08;
	// 	break;
	// case 1:
	// 	IOREG[(REG_IF + 0) - 0x4000000] |= 0x10;
	// 	break;
	// case 2:
	// 	IOREG[(REG_IF + 0) - 0x4000000] |= 0x20;
	// 	break;
	// case 3:
	// 	IOREG[(REG_IF + 0) - 0x4000000] |= 0x40;
	// 	break;
	// }
}