#pragma once

#include <array>

using u8  = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

class Bus;

class Tmr
{
    private:

	std::array<bool, 4> prevTmrState;
	std::array<bool, 4> currTmrState;
	std::array<bool, 4> tmrOverflow;
	std::array<u16, 4> tmrCounter;
	std::array<u16, 4> tmrReload;
    std::array<u16, 4> timerLUT;

    Bus* bus_ptr;

    public:

    Tmr();

    void ConnectBus(Bus*);
    
    template<typename T>
    void tmrWriteIO(const u32&, const T);

    template<typename T>
    void tmrReadIO(const u32&, const T);

   	void stepTimers();
	void tmrInterrupt(int);

};