#pragma once
#include <array>
#include <cassert>
#include <iostream>
#include "capstone/capstone.h"
#include "capstone/platform.h"

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

constexpr unsigned int STATE = 0x00000020;
constexpr unsigned int FIQ = 0x00000040;
constexpr unsigned int IRQ = 0x00000080;
constexpr unsigned int OVRFLW = 0x10000000;
constexpr unsigned int CARRY = 0x20000000;
constexpr unsigned int ZERO = 0x40000000;
constexpr unsigned int NEG = 0x80000000;

unsigned int countSetBits(uint16_t n);

#define assert_msg(x) !(std::cerr << "Assertion failed: " << x << std::endl)

class Gui;

class Bus;

#define CODE "\x55\x48\x8b\x05\xb8\x13\x00\x00"

class Arm
{
private:

    Bus* busPtr;

    u32 cpsr;

    csh handle_arm, handle_thumb;
    cs_insn* insn_arm, insn_thumb;
    size_t count_arm, count_thumb;

    //std::array<u32, 16> system;
    //std::array<u32, 7>  fiq_arm;
    //std::array<u32, 2>  svc_arm;
    //std::array<u32, 2>  abt_arm;
    //std::array<u32, 2>  und_arm;
    //std::array<u32, 7>  fiq_thumb;
    //std::array<u32, 2>  svc_thumb;
    //std::array<u32, 2>  abt_thumb;
    //std::array<u32, 2>  und_thumb;
    //std::array<u32, 5>  spsr;

    u32 system[16];
    u32 fiq[7];
    u32 supervisor[2];
    u32 abort[2];
    u32 irq[2];
    u32 undefined[2];

    u32 spsr[5];

    void (Arm::* InstructionAddress)(void);

    u32 pipeline[2];

    friend class Gui;

public:
    u32 opcode;

    Arm();

    void ConnectBus(Bus* ptr);

    u8  CpuRead(u32);
    u32 CpuRead16(u32);
    u32 CpuRead32(u32);
    void CpuWrite(u32, u8);
    void CpuWrite16(u32, u16);
    void CpuWrite32(u32, u32);

    u32 ReadFromPC();
    void WriteToPC(u32);
    bool ConditionField(u8);
    u32 ReadRegisterFlag();
    u32 ReadRegisterThumb(u8);
    u32 ReadRegisterArm(u8, bool = 0);
    void SetRegisterFlag(u32);
    void SetRegisterThumb(u8, u32);
    void SetRegisterArm(u8, u32, bool = 0);

    void set_neg(bool s) { s ? cpsr |= NEG : cpsr &= (~NEG); }
    void set_ovr(bool s) { s ? cpsr |= OVRFLW : cpsr &= (~OVRFLW); }
    void set_car(bool s) { s ? cpsr |= CARRY : cpsr &= (~CARRY); }
    void set_zer(bool s) { s ? cpsr |= ZERO : cpsr &= (~ZERO); }
    void set_fiq(bool s) { s ? cpsr |= FIQ : cpsr &= (~FIQ); }
    void set_irq(bool s) { s ? cpsr |= IRQ : cpsr &= (~IRQ); }
    void set_sta(bool s) { s ? cpsr |= STATE : cpsr &= (~STATE); }

    bool neg_res() { return (cpsr & NEG) ? 1 : 0; }
    bool car_res() { return (cpsr & CARRY) ? 1 : 0; }
    bool zer_res() { return (cpsr & ZERO) ? 1 : 0; }
    bool fiq_res() { return (cpsr & FIQ) ? 1 : 0; }
    bool irq_res() { return (cpsr & IRQ) ? 1 : 0; }
    bool sta_res() { return (cpsr & STATE) ? 1 : 0; }
    bool ovr_res() { return (cpsr & OVRFLW) ? 1 : 0; }

    void BarrelShifter(u32&, u32&, u32&, u32&, u32&, bool = true);
    void BarrelShifterImmediate(u32&, u32&, u32&, u32&, u32&, bool = true);
    void DoShifting(u32&, u32&, u32&, u32&);
    void DoLoadFlag(u8&, u8&, u32&, u8&);
    void DoLoadHalfword(u8&, u8&, u8&, u8&, u32&);
    void calc_carry(u32, u32);
    void calc_overflow(u32, u32, u32);
    void calc_neg(u32);
    u32 SignExtend(u32, u32);
    u32 RotateMisallignedWord(u32);
    u32 RotateMisallignedHalfWord(u32);

    void DoNothing();

    void DataProcessing();
    void MRS();
    void MSR();
    void Multiply();
    void MultiplyLong();
    void SingleDataSwap();
    void BranchAndExchange();
    void HalfwordTransferReg();
    void HalfwordTransferImm();
    void SingleDataTransfer();
    void Undefined();
    void BlockDataTransfer();
    void Branch();
    void CoprocessorDataTransfer();
    void CoprocessorDataOperation();
    void CoprocessorRegisterTransfer();

    void MoveShiftedRegister();
    void AddSubtract();
    void MovCompAddSubImm();
    void AluOperations();
    void BranchExchange();
    void PcRelativeLoad();
    void LoadStoreRegOffset();
    void LoadStoreSignExtend();
    void LoadStoreImmOffset();
    void LoadStoreHalfword();
    void SpRelativeLoadStore();
    void LoadAddress();
    void AddOffsetToStackPointer();
    void PushPopRegisters();
    void MultipleLoadStore();
    void ConditionalBranch();
    void UnconditionalBranch();
    void LongBranchWithLink();

    void InterruptRequest();
    void SoftwareInterrupt();

    u32 fetch();
    void decodeArm();
    void decodeThumb();
    void flushPipeline();
    void resetCpu();
    void runCpu(int);
    u32  stepCpu();

    ~Arm();
};
