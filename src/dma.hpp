#pragma once

#include <array>

using u8 =  std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

class Bus;

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

//
//typedef struct DMA_CNT_BITS
//{
//	u8 padding_5 : 5;
//	u8 adjust_des : 2;
//	u8 adjust_src : 2;
//	u8 repeat : 1;
//	u8 chunk_size : 1;
//	u8 padding_1 : 1;
//	u8 timing : 2;
//	u8 interrupt : 1;
//	u8 enable : 1;
//
//} DMA_CNT_BITS;

class Dma
{

private:

	bool doTransfer = false;

	std::array<bool, 4> prevVblank;
	std::array<bool, 4> currVblank;
	std::array<bool, 4> prevHblank;
	std::array<bool, 4> currHblank;
	std::array<bool, 4> prevState;
	std::array<bool, 4> currState;
	std::array<u32, 4> transferNum;
	std::array<u32, 4> destAddr;
	std::array<u32, 4> srcAddr;
	std::array<DmaData, 4> DmaChannels;

	Bus* busPtr;

public:

	Dma();
	void ConnectBus(Bus*);
	void GetDmaData();
	void CheckAvailableDMA(u8);
	void DoDmaCalculations(int channel);
	void DoDma();
	void doDmaInterrupt(int channel);

};