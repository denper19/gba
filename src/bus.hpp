#pragma once

#include <string>
#include "mem.hpp"
#include "cpu.hpp"
#include "ppu.hpp"

#define KB 1024
#define CYCLES_PER_FRAME (240 + 68) * (160 + 68) * 4

#define DMA_CNT_H_0 0xeda
#define DMA_CNT_H_1 0xaefa
#define DMA_CNT_H_2 0xdadaf
#define DMA_CNT_H_3 0xfa

#define TMR_CNT_H_0 0xeda
#define TMR_CNT_H_1 0xaefa
#define TMR_CNT_H_2 0xdadaf
#define TMR_CNT_H_3 0xfa

typedef struct DmaData
{
	int dma_dest_adjust;
	int dma_src_adjust;
	int dma_repeat;
	int dma_cs;
	int dma_tm;
	int dma_int;
	int dma_en;

} DmaData;

typedef struct DMA_CNT_BITS
{
	u8 padding_5 : 5;
	u8 adjust_des : 2;
	u8 adjust_src : 2;
	u8 repeat : 1;
	u8 chunk_size : 1;
	u8 padding_1 : 1;
	u8 timing : 2;
	u8 interrupt : 1;
	u8 enable : 1;

} DMA_CNT_BITS;

class Bus
{
private:
	int timerLUT[4];
	std::array<bool, 4> prevVblank;
	std::array<bool, 4> currVblank;
	std::array<bool, 4> prevHblank;
	std::array<bool, 4> currHblank;
	std::array<bool, 4> prevTmrState;
	std::array<bool, 4> currTmrState;
	std::array<bool, 4> tmrOverflow;
	std::array<bool, 4> prevState;
	std::array<bool, 4> currState;
	std::array<u16, 4> tmrCounter;
	std::array<u16, 4> tmrReload;
	std::array<u32, 4>transferNum;
	std::array<u32, 4>destAddr;
	std::array<u32, 4>srcAddr;
	std::array<DmaData, 4> DmaChannels;

	Arm* cpuPtr;
	Lcd* lcdPtr;
	bool HALT_CPU = false;
	bool IS_THE_CPU_IN_HALT = false;

public:
	bool doTransfer, inHblank, inVblank;

	Mem BIOS, IWRAM, EWRAM, IOREG, COLOR,
		VRAM, OAM, PAK1, PAK2, PAK3, SRAM;

	Mem m_bios, m_iwram, m_ewram, m_mmio, m_color,
		m_vram, m_oam, m_pak, m_sram;

	Bus();

	void ConnectCPU(Arm*);
	void ConnectPPU(Lcd*);

	u8   BusRead(u32);
	u32	 BusRead16(u32);
	u32	 BusRead32(u32);
	void BusWrite(u32, u8);
	void BusWrite16(u32, u16);
	void BusWrite32(u32, u32);

	//DMA
	void GetDmaData();
	void DoDmaCalculations(int channel);
	void DoDma();
	void doDmaInterrupt(int channel);

	//Timers
	void DoTimers();
	void doTimerInterrupt(int);

	void BusWriteToCol();
	void BusReadFromCol();
	void BusWriteToOam();
	void BusReadFromOam();
	void BusWriteToVram();
	void BusReadFromVram();

	void setVblank();
	void setHblank();
	void clrVblank();
	void clrHblank();
	void HandleInterrupts();
	void stepSystem();
	void write_controls(uint8_t*);

	void write_mmio(u32 addr, u32 data, u8);

	void Run();

	~Bus();
};

