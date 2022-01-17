#include "cpu.hpp"

#include "bus.hpp"

/*------------------OTHER------------------------------------------------------------*/

unsigned int countSetBits(uint16_t n) {
	unsigned int count = 0;
	while (n)
	{
		n &= (n - 1);
		count++;
	}
	return count;
}

/*-----------------------GENERAL FUNCTIONS--------------------------------------------*/

Arm::Arm()
{
	busPtr = NULL;
	//MyFile.open("log2.bin");
	//cs_open(CS_ARCH_ARM, CS_MODE_ARM, &handle_arm);
	//cs_open(CS_ARCH_ARM, CS_MODE_THUMB, &handle_thumb);
	resetCpu();
}

void Arm::ConnectBus(Bus* ptr)
{
	busPtr = ptr;
}

/*-------------------------------READ/WRITE FUNCTIONS------------------------------------*/

void Arm::CpuWrite(u32 addr, u8 data)
{
	if ((addr >= 0x4000000) && (addr <= 0x4000300))
		busPtr->write_mmio(addr, data, 2);
	else
		busPtr->BusWrite(addr, data);
}

u8 Arm::CpuRead(u32 addr)
{
	if (addr == 0x0E000000)
		return 0x62;
	if (addr == 0x0E000001)
		return 0x13;
	return busPtr->BusRead(addr);
}

void Arm::CpuWrite16(u32 addr, u16 data)
{
	addr &= ~1;
	u8 byte1 = (data >> 0) & 0xFF;
	u8 byte2 = (data >> 8) & 0xFF;
	if ((addr >= 0x4000000) && (addr < 0x4000400))
	{
		busPtr->write_mmio(addr, data, 1);
	}
	else
	{
		busPtr->BusWrite(addr + 0, byte1);
		busPtr->BusWrite(addr + 1, byte2);
	}
}

u32 Arm::CpuRead16(u32 addr)
{
	//assert(false && 'Shouldn\'t read from an unaligned halfword');

	u8 byte1 = busPtr->BusRead(addr + 0);
	u8 byte2 = busPtr->BusRead(addr + 1);
	u32 value = (byte2 << 8) | (byte1);
	return value;
}

void Arm::CpuWrite32(u32 addr, u32 data)
{
	addr &= ~3;
	u8 byte1 = (data >> 0) & 0xFF;
	u8 byte2 = (data >> 8) & 0xFF;
	u8 byte3 = (data >> 16) & 0xFF;
	u8 byte4 = (data >> 24) & 0xFF;
	if ((addr >= 0x4000000) && (addr < 0x4000400))
	{
		busPtr->write_mmio(addr, data, 0);
	}
	else
	{
		busPtr->BusWrite(addr + 0, byte1);
		busPtr->BusWrite(addr + 1, byte2);
		busPtr->BusWrite(addr + 2, byte3);
		busPtr->BusWrite(addr + 3, byte4);
	}
}

u32 Arm::CpuRead32(u32 addr)
{
	//assert(false && 'Shouldn\'t read from an unaligned haword');
	u8 byte1 = busPtr->BusRead(addr + 0);
	u8 byte2 = busPtr->BusRead(addr + 1);
	u8 byte3 = busPtr->BusRead(addr + 2);
	u8 byte4 = busPtr->BusRead(addr + 3);
	u32 value = (byte4 << 24) | (byte3 << 16) | (byte2 << 8) | (byte1);
	return value;
}
void Arm::SetRegisterFlag(u32 data)
{
	switch (cpsr & 0x1F)
	{
	case 0x10: cpsr = data & ~0xFF; return; //user mode can't change control bits
	case 0x11: spsr[0] = data; return;
	case 0x12: spsr[1] = data; return;
	case 0x13: spsr[2] = data; return;
	case 0x17: spsr[3] = data; return;
	case 0x1B: spsr[4] = data; return;
	case 0x1F: cpsr = data; return;
	default:
		printf("\n There was an error in Arm::SetRegisterFlag, \
			the correct cpsr mode was not found. The value was: 0x%x \n", cpsr & 0x1F);
		return;
	};
}

u32 Arm::ReadRegisterFlag()
{
	switch (cpsr & 0x1F)
	{
	case 0x10: return cpsr;
	case 0x11: return spsr[0];
	case 0x12: return spsr[1];
	case 0x13: return spsr[2];
	case 0x17: return spsr[3];
	case 0x1B: return spsr[4];
	case 0x1F: return cpsr;
	default:
		printf("\n There was an error in Arm::ReadRegisterFlag, \
			the correct cpsr mode was not found. The value was: 0x%x \n", cpsr & 0x1F);
		return 0;
	};
}

u32 Arm::ReadRegisterThumb(u8 reg)
{
	//assert((sta_res()) && "Reading from THUMB in ARM mode");

	if (reg < 8) return system[reg];
	else if (reg == 15) return system[15];

	switch (cpsr & 0x1F)
	{
	case 0x10: return system[reg];
	case 0x11: return fiq[reg - 13];
	case 0x12: return irq[reg - 13];
	case 0x13: return supervisor[reg - 13];
	case 0x17: return abort[reg - 13];
	case 0x1B: return undefined[reg - 13];
	case 0x1F: return system[reg];
	default:
		printf("\n There was an error in Arm::ReadRegisterThumb, \
			the correct cpsr mode was not found. The value was: 0x%x \n", cpsr & 0x1F);
		return 0;
	}
}

u32 Arm::ReadRegisterArm(u8 reg, bool force_usr_mode)
{
	//assert((!sta_res()) && "Reading from ARM in THUMB mode");

	u32 temp_cpsr = force_usr_mode ? 0x10 : cpsr & 0x1F;

	if (reg == 15) return system[15];

	switch (temp_cpsr)
	{
	case 0x10: return system[reg];
	case 0x11:

		if (reg < 8)
			return system[reg];
		else if (reg >= 8)
			return fiq[reg - 8];

	case 0x12:

		if (reg < 13)
			return system[reg];
		else if (reg >= 13)
			return irq[reg - 13];

	case 0x13:

		if (reg < 13)
			return system[reg];
		else if (reg >= 13)
			return supervisor[reg - 13];

	case 0x17:

		if (reg < 13)
			return system[reg];
		else if (reg >= 13)
			return abort[reg - 13];

	case 0x1B:

		if (reg < 13)
			return system[reg];
		else if (reg >= 13)
			return undefined[reg - 13];

	case 0x1F: return system[reg];
	default:
		printf("\n There was an error in Arm::ReadRegisterArm, \
the correct cpsr mode was not found. The value was: 0x%x OPCODE: 0x%X\n", cpsr & 0x1F, opcode);
		return 0;
	}
}

void Arm::WriteToPC(u32 data)
{
	if (sta_res())
	{
		//THUMB mode
		system[15] = data & 0xFFFFFFFE;
		flushPipeline();
		system[15] -= 2;
	}
	else
	{
		//ARM mode
		system[15] = data & 0xFFFFFFFC;
		flushPipeline();
		system[15] -= 4;
	}
}

u32 Arm::ReadFromPC()
{
	return system[15];
}

void Arm::SetRegisterThumb(u8 reg, u32 data)
{
	//assert((sta_res()) && "Writing to THUMB in ARM mode");


	if (reg < 8)
	{
		system[reg] = data;
		return;
	}
	else if (reg == 15)
	{
		WriteToPC(data);
		return;
	}

	switch (cpsr & 0x1F)
	{
	case 0x10: system[reg] = data; return;
	case 0x11: fiq[reg - 13] = data; return;
	case 0x12: irq[reg - 13] = data; return;
	case 0x13: supervisor[reg - 13] = data; return;
	case 0x17: abort[reg - 13] = data; return;
	case 0x1B: undefined[reg - 13] = data; return;
	case 0x1F: system[reg] = data; return;
	default:
		printf("\n There was an error in Arm::SetRegisterThumb, \
			the correct cpsr mode was not found. The value was: 0x%x \n", cpsr & 0x1F);
		return;
	}

}

void Arm::SetRegisterArm(u8 reg, u32 data, bool force_usr_mode)
{
	u32 temp_cpsr = force_usr_mode ? 0x10 : cpsr & 0x1F;
	//assert((!sta_res()) && "Writing to ARM in THUMB mode");

	if (reg == 15)
	{
		WriteToPC(data);
		return;
	}

	switch (temp_cpsr)
	{
	case 0x10: system[reg] = data; break;
	case 0x11:

		if (reg < 8)
			system[reg] = data;
		else if (reg >= 8)
			fiq[reg - 8] = data;
		return;

	case 0x12:

		if (reg < 13)
			system[reg] = data;
		else if (reg >= 13)
			irq[reg - 13] = data;
		return;

	case 0x13:

		if (reg < 13)
			system[reg] = data;
		else if (reg >= 13)
			supervisor[reg - 13] = data;
		return;

	case 0x17:

		if (reg < 13)
			system[reg] = data;
		else if (reg >= 13)
			abort[reg - 13] = data;
		return;

	case 0x1B:

		if (reg < 13)
			system[reg] = data;
		else if (reg >= 13)
			undefined[reg - 13] = data;
		return;

	case 0x1F: system[reg] = data; return;
	default:
		printf("\n There was an error in Arm::SetRegisterArm, \
			the correct cpsr mode was not found. The value was: 0x%x \n", cpsr & 0x1F);
		return;
	}
}

bool Arm::ConditionField(u8 select)
{
	switch (select)
	{
	case 0: return  zer_res();
	case 1: return  !zer_res();
	case 2: return  car_res();
	case 3: return  !car_res();
	case 4: return  neg_res();
	case 5: return  !neg_res();
	case 6: return  ovr_res();
	case 7: return  !ovr_res();
	case 8: return  car_res() && !zer_res();
	case 9: return  !car_res() || zer_res();
	case 10: return neg_res() == ovr_res();
	case 11: return neg_res() != ovr_res();
	case 12: return !zer_res() && (neg_res() == ovr_res());
	case 13: return zer_res() || (neg_res() != ovr_res());
	case 14: return true;
	default:
		printf("\n At pc: 0x%X\n", system[15]);
		printf("\n There was an error in Arm::ConditionField, \
the correct cpsr mode was not found. The value was: 0x%x. returned 'FALSE.' \n", select);
		//assert(0);
		return false;
	}
}

/*--------------------------------MISC FUNCTIONS------------------------------------------*/

void Arm::BarrelShifter(u32& value, u32& shift_type, u32& shift_amount, u32& result, u32& carry_out, bool disable_rrx)
{
	switch (shift_type)
	{
		//Logical Left
	case 0x00:
	{
		if (shift_amount == 0)
		{
			result = value;
			carry_out = car_res();
		}
		else if (shift_amount < 32)
		{
			result = value << shift_amount;
			carry_out = value & (0x1 << (32 - shift_amount));
		}
		else if (shift_amount == 32)
		{
			result = 0;
			carry_out = value & 0x1;
		}
		else
		{
			result = 0;
			carry_out = 0;
		}

		break;
	}
	//Logical Right
	case 0x01:
	{
		result = value >> shift_amount;
		carry_out = value & (0x1 << (shift_amount - 1));
		if (shift_amount == 0)
		{
			result = value;
			carry_out = car_res();
		}
		else if (shift_amount < 32)
		{
			result = value >> shift_amount;
			carry_out = value & (0x1 << (shift_amount - 1));
		}
		else if (shift_amount == 32)
		{
			result = 0;
			carry_out = value & 0x80000000;
		}
		else if (shift_amount > 32)
		{
			result = 0;
			carry_out = 0;
		}

		break;
	}
	//Arithmetic Right
	case 0x02:
	{

		if (shift_amount == 0x00)
		{
			result = value;
			carry_out = car_res();
		}
		else if (shift_amount < 32)
		{
			int32_t r = value;
			r >>= shift_amount;
			result = r;
			carry_out = value & (0x1 << (shift_amount - 1));
		}
		else if (shift_amount >= 32)
		{
			if (value & 0x80000000)
			{
				result = 0xFFFFFFFF;
				carry_out = value & 0x80000000;
			}
			else
			{
				result = 0;
				carry_out = value & 0x80000000;
			}
		}
		break;
	}
	//Rotate Right
	case 0x03:
	{
		if (shift_amount == 0x00)
		{
			result = value;
			carry_out = car_res();
		}
		else if ((shift_amount & 0x1F) == 0x00)
		{
			result = value;
			carry_out = value & 0x80000000;
		}
		else if ((shift_amount & 0x1F) > 0x00)
		{
			shift_amount %= 32;//get to [0, 31] range if > 32
			result = (value >> shift_amount) | (value << (32 - shift_amount));
			carry_out = value & (0x1 << (shift_amount - 1));
		}

		break;
	}
	}
}

void Arm::BarrelShifterImmediate(u32& value, u32& shift_type, u32& shift_amount, u32& result, u32& carry_out, bool disable_rrx)
{
	switch (shift_type)
	{
		//Logical Left
	case 0x00:
	{
		if (shift_amount == 0)
		{
			result = value;
			carry_out = car_res();
		}
		else if (shift_amount > 0)
		{
			result = value << shift_amount;
			carry_out = value & (0x1 << (32 - shift_amount));
		}

		break;
	}
	//Logical Right
	case 0x01:
	{

		if (shift_amount == 0)
		{
			result = 0;
			carry_out = value & 0x80000000;
		}
		else if (shift_amount > 0)
		{
			result = value >> shift_amount;
			carry_out = value & (0x1 << (shift_amount - 1));
		}

		break;
	}
	//Arithmetic Right
	case 0x02:
	{
		if (shift_amount == 0x00)
		{
			if (value & 0x80000000)
			{
				result = 0xFFFFFFFF;
				carry_out = value & 0x80000000;
			}
			else
			{
				result = 0x0;
				carry_out = value & 0x80000000;
			}
		}
		else if (shift_amount > 0x00)
		{
			int32_t r = value;
			r >>= shift_amount;
			result = r;
			carry_out = value & (0x1 << (shift_amount - 1));
		}

		break;
	}
	//Rotate Right
	case 0x03:
	{
		if ((shift_amount == 0x00) && (!disable_rrx))
		{
			//RRX
			result = (value >> 1) | (car_res() << 31);
			carry_out = value & 0x1;
		}
		else if (shift_amount > 0x00)
		{
			shift_amount %= 32;
			result = (value >> shift_amount) | (value << (32 - shift_amount));
			carry_out = value & (0x1 << (shift_amount - 1));
		}
		break;
	}
	}
}

void Arm::DoShifting(u32& immediate, u32& operand2, u32& result, u32& carry_out)
{
	if (!immediate)
	{
		bool shift_version = (operand2 >> 4) & 0x1;

		u32 shift_type = (operand2 >> 5) & 0x3;

		u32 shift_amount = 0;

		u32 value = 0;

		if (shift_version) {
			u8 rs = (operand2 >> 8) & 0xF;
			shift_amount = ReadRegisterArm(rs) & 0xFF;
			value = ReadRegisterArm(operand2 & 0xF);
			BarrelShifter(value, shift_type, shift_amount, result, carry_out);
		}
		else
		{
			shift_amount = (operand2 >> 7) & 0x1F;
			value = ReadRegisterArm(operand2 & 0xF);
			BarrelShifterImmediate(value, shift_type, shift_amount, result, carry_out, false);
		}
	}
	else
	{
		u32 imm = operand2 & 0xFF;

		u32 rotate_imm = (operand2 >> 8) & 0xF;

		rotate_imm *= 2;

		result = (imm >> (rotate_imm)) | (imm << (32 - rotate_imm));

		if (rotate_imm == 0)
			carry_out = car_res();
		else
			carry_out = result & 0x80000000;
	}
}

void Arm::DoLoadFlag(u8& load, u8& byte, u32& final_address, u8& src_register)
{
	u32 value = 0;
	if (load)
	{
		if (!byte)
		{
			value = RotateMisallignedWord(final_address);
			SetRegisterArm(src_register, value);
		}
		else
		{
			value = CpuRead(final_address);
			SetRegisterArm(src_register, value);
		}
	}
	else
	{
		if (!byte)
		{
			value = ReadRegisterArm(src_register);
			if (src_register == 15)
				value += 4;
			CpuWrite32(final_address, value);
		}
		else
		{
			value = ReadRegisterArm(src_register);
			if (src_register == 15)
				value += 4;
			CpuWrite(final_address, ReadRegisterArm(src_register) & 0xFF);
		}
	}
}

void Arm::DoLoadHalfword(u8& l, u8& h, u8& s, u8& rd, u32& final_address)
{
	u32 value = 0;
	if (l)
	{
		if (h)
		{
			value = RotateMisallignedHalfWord(final_address);
			if (s)
			{
				if (final_address & 1) value = SignExtend(CpuRead(final_address), 24);
				else value = SignExtend(CpuRead16(final_address), 16);
			}
		}
		else
		{
			value = CpuRead(final_address);
			if (s) value = SignExtend(value, 24);
		}
		SetRegisterArm(rd, value);
	}
	else
	{
		if (h)
		{
			value = ReadRegisterArm(rd) & 0xFFFF;
			CpuWrite16(final_address, value);
		}
		else
		{
			value = ReadRegisterArm(rd) & 0xFF;
			CpuWrite(final_address, value);
		}
	}
}

void Arm::calc_carry(u32 A, u32 B)
{
	//Need to manually promot operands to 64 because compiler only does it internally to 32 bits
	set_car(((u64)A + (u64)B) > 0xFFFFFFFF);
}

void Arm::calc_overflow(u32 A, u32 B, u32 R)
{
	//if (B > A) std::swap(A, B);
	set_ovr((!((A ^ B) & 0x80000000)) && ((A ^ R) & 0x80000000));
}

void Arm::calc_neg(u32 A)
{
	set_neg(A & 0x80000000);
}

u32 Arm::SignExtend(u32 value, u32 extend)
{
	int32_t result = value << extend;
	result >>= extend;
	return result;
}

u32 Arm::RotateMisallignedWord(u32 addr)
{
	const auto rotate = (addr & 3) * 8;
	addr &= ~3;
	u32 value = CpuRead32(addr);
	if (rotate) value = (value << (32 - rotate)) | (value >> rotate);
	return value;
}

u32 Arm::RotateMisallignedHalfWord(u32 addr)
{
	const auto rotate = addr & 1;
	addr &= ~1;
	u32 value = CpuRead16(addr);
	if (rotate) value = (value << (32 - 8)) | (value >> 8);
	return value;
}

/*-------------------------ARM OPCODES-------------------------------------------------------------*/

void Arm::DoNothing()
{
	//Literally do nothing like a NOP instruction
}

void Arm::DataProcessing()
{
	u32 operand2 = opcode & 0xFFF;
	u8 rd = (opcode >> 12) & 0xF;
	u8 rn = (opcode >> 16) & 0xF;
	bool s = (opcode >> 20) & 0x1;
	u8 op = (opcode >> 21) & 0xF;
	u32 immediate = (opcode >> 25) & 0x1;

	u32 result = 0;
	u32 final_result = 0;
	u32 carry_out = 0;

	DoShifting(immediate, operand2, result, carry_out);

	u32 operand1 = ReadRegisterArm(rn);

	bool write = true;

	if (!immediate && (opcode & 0x10))
	{
		if (rn == 15) operand1 += 4;
		if ((operand2 & 0xF) == 15) result += 4;
	}

	switch (op)
	{
	case 0x00: {
		//AND
		final_result = result & operand1;

		if (s && (rd == 15))
		{
			cpsr = ReadRegisterFlag();
			WriteToPC(final_result);
			s = 0;
		}
		else if (s)
		{
			set_car(carry_out);
			calc_neg(final_result);
			set_zer(final_result == 0);
		}

		break;
	}
	case 0x01:
	{
		//EOR
		final_result = result ^ operand1;

		if (s && (rd == 15))
		{
			cpsr = ReadRegisterFlag();
			WriteToPC(final_result);
			s = 0;
		}
		else if (s)
		{
			set_car(carry_out);
			calc_neg(final_result);
			set_zer(final_result == 0);
		}

		break;
	}
	case 0x02: {
		//SUB

		final_result = operand1 - result;

		if (s && (rd == 15))
		{
			cpsr = ReadRegisterFlag();
			WriteToPC(final_result);
			s = 0;
		}
		else if (s)
		{
			set_car(operand1 >= result);
			calc_neg(final_result);
			set_zer(final_result == 0);
			calc_overflow(operand1, ~result, final_result);
		}

		break;
	}
	case 0x03: {
		//RSB

		final_result = result - operand1;

		if (s && (rd == 15))
		{
			cpsr = ReadRegisterFlag();
			WriteToPC(final_result);
			s = 0;
		}
		else if (s)
		{
			set_car(result >= operand1);
			calc_neg(final_result);
			set_zer(final_result == 0);
			calc_overflow(result, ~operand1, final_result);
		}

		break;
	}
	case 0x04: {
		//ADD
		final_result = result + operand1;

		if (s && (rd == 15))
		{
			cpsr = ReadRegisterFlag();
			WriteToPC(final_result);
			s = 0;
		}
		else if (s)
		{
			set_car(operand1 > (0xFFFFFFFF - result));
			calc_neg(final_result);
			set_zer(final_result == 0);
			calc_overflow(operand1, result, final_result);
		}

		break;
	}
	case 0x05: {
		//ADC

		final_result = result + operand1 + car_res();

		if (s && (rd == 15))
		{
			cpsr = ReadRegisterFlag();
			WriteToPC(final_result);
			s = 0;
		}
		else if (s)
		{
			calc_carry(result, operand1 + car_res());
			set_car(((u64)result + (u64)operand1 + (u64)car_res()) > 0xFFFFFFFF);
			calc_neg(final_result);
			set_zer(final_result == 0);
			calc_overflow(operand1, result + car_res(), final_result);
		}

		break;
	}
	case 0x06: {
		//SBC

		final_result = operand1 - result - !car_res();

		if (s && (rd == 15))
		{
			cpsr = ReadRegisterFlag();
			WriteToPC(final_result);
			s = 0;
		}
		else if (s)
		{
			set_car((u64)operand1 >= ((u64)result + (u64)!car_res()));
			calc_neg(final_result);
			set_zer(final_result == 0);
			calc_overflow(operand1, ~(result + !car_res()), final_result);
		}

		break;
	}
	case 0x07: {
		//RSC

		final_result = result - operand1 - !car_res();

		if (s && (rd == 15))
		{
			cpsr = ReadRegisterFlag();
			WriteToPC(final_result);
			s = 0;
		}
		else if (s)
		{
			set_car(result >= (operand1 + !car_res()));
			calc_neg(final_result);
			set_zer(final_result == 0);
			calc_overflow(result, ~(operand1 + !car_res()), final_result);
		}

		break;
	}
	case 0x08: {
		//TST
		final_result = result & operand1;

		set_car(carry_out);
		calc_neg(final_result);
		set_zer(final_result == 0);

		write = false;

		if (rd == 15) {
			if ((cpsr & 0x1F) == 0x10)
				cpsr = ReadRegisterFlag() & ~0xFF;
			else
				cpsr = ReadRegisterFlag();
		}

		break;
	}
	case 0x09: {
		//TEQ

		final_result = result ^ operand1;

		set_car(carry_out);
		calc_neg(final_result);
		set_zer(final_result == 0);

		write = false;

		if (rd == 15) {
			if ((cpsr & 0x1F) != 0x10)
				cpsr = ReadRegisterFlag();
		}

		break;
	}
	case 0x0a: {
		//CMP
		final_result = operand1 - result;

		set_car(operand1 >= result);
		calc_neg(final_result);
		set_zer(final_result == 0);
		calc_overflow(operand1, ~result, final_result);

		write = false;

		if (rd == 15) {
			if ((cpsr & 0x1F) == 0x10)
				cpsr = ReadRegisterFlag() & ~0xFF;
			else
				cpsr = ReadRegisterFlag();
		}

		break;
	}
	case 0x0b:
	{
		//CMN
		final_result = result + operand1;

		set_car(operand1 >= (0xFFFFFFFF - result));
		calc_neg(final_result);
		set_zer(final_result == 0);
		calc_overflow(operand1, result, final_result);

		write = false;

		if (rd == 15) {
			if ((cpsr & 0x1F) == 0x10)
				cpsr = ReadRegisterFlag() & ~0xFF;
			else
				cpsr = ReadRegisterFlag();
		}

		break;
	}
	case 0x0c:
	{
		//ORR
		final_result = result | operand1;

		if (s && (rd == 15))
		{
			cpsr = ReadRegisterFlag();
			WriteToPC(final_result);
			s = 0;
		}
		else if (s)
		{
			set_car(carry_out);
			calc_neg(final_result);
			set_zer(final_result == 0);
		}

		break;
	}
	case 0x0d: {

		//MOV

		final_result = result;

		if (s && (rd == 15))
		{
			cpsr = ReadRegisterFlag();
			WriteToPC(final_result);
			s = 0;
		}
		else if (s)
		{
			set_car(carry_out);
			calc_neg(final_result);
			set_zer(final_result == 0);
		}

		break;
	}
	case 0x0e:
	{
		//BIC
		final_result = operand1 & ~result;

		if (s && (rd == 15))
		{
			cpsr = ReadRegisterFlag();
			WriteToPC(final_result);
			s = 0;
		}
		else if (s)
		{
			set_car(carry_out);
			calc_neg(final_result);
			set_zer(final_result == 0);
		}

		break;
	}
	case 0x0f: {
		//MVN
		final_result = ~result;

		if (s && (rd == 15))
		{
			cpsr = ReadRegisterFlag();
			WriteToPC(final_result);
			s = 0;
		}
		else if (s)
		{
			set_car(carry_out);
			calc_neg(final_result);
			set_zer(final_result == 0);
		}

		break;
	}
	}

	if (write) SetRegisterArm(rd, final_result);
}

void Arm::MRS()
{
	u8 rd = (opcode >> 12) & 0xF;
	u8 ps = (opcode >> 22) & 0x1;
	ps ? SetRegisterArm(rd, ReadRegisterFlag()) : SetRegisterArm(rd, cpsr);
}

void Arm::MSR()
{
	u8 I = (opcode >> 25) & 0x1;
	u8 R = (opcode >> 22) & 0x1;
	u8 field_mask = (opcode >> 16) & 0xF;

	u32 operand = 0;

	if (I == 1)
	{
		u8 imm_value = opcode & 0xFF;
		u8 rotate_imm = (opcode >> 8) & 0xF;
		rotate_imm *= 2;
		operand = (imm_value >> rotate_imm) | (imm_value << (32 - rotate_imm));
	}
	else
	{
		u8 rm = opcode & 0xF;
		operand = ReadRegisterArm(rm);
	}

	if (R == 0)
	{
		if ((field_mask & 0x1) && ((cpsr & 0x1F) != 0x10))
			cpsr = (cpsr & 0xFFFFFF00) | (operand & 0xFF);

		if ((field_mask & 0x2) && ((cpsr & 0x1F) != 0x10))
			cpsr = (cpsr & 0xFFFF00FF) | (operand & 0xFF00);

		if ((field_mask & 0x4) && ((cpsr & 0x1F) != 0x10))
			cpsr = (cpsr & 0xFF00FFFF) | (operand & 0xFF0000);

		if ((field_mask & 0x8) && ((cpsr & 0x1F) != 0x10))
			cpsr = (cpsr & 0x00FFFFFF) | (operand & 0xFF000000);

	}
	else
	{

		if ((field_mask & 0x1) && ((cpsr & 0x1F) != 0x10) && ((cpsr & 0x1F) != 0x1F))
			SetRegisterFlag((ReadRegisterFlag() & 0xFFFFFF00) | (operand & 0xFF));

		if ((field_mask & 0x2) && ((cpsr & 0x1F) != 0x10) && ((cpsr & 0x1F) != 0x1F))
			SetRegisterFlag((ReadRegisterFlag() & 0xFFFF00FF) | (operand & 0xFF00));

		if ((field_mask & 0x4) && ((cpsr & 0x1F) != 0x10) && ((cpsr & 0x1F) != 0x1F))
			SetRegisterFlag((ReadRegisterFlag() & 0xFF00FFFF) | (operand & 0xFF0000));

		if ((field_mask & 0x8) && ((cpsr & 0x1F) != 0x10) && ((cpsr & 0x1F) != 0x1F))
			SetRegisterFlag((ReadRegisterFlag() & 0x00FFFFFF) | (operand & 0xFF000000));

	}
}

void Arm::Multiply()
{
	//MUL

	uint8_t rm = opcode & 0xF;
	uint8_t rs = (opcode >> 8) & 0xF;
	uint8_t rn = (opcode >> 12) & 0xF;
	uint8_t rd = (opcode >> 16) & 0xF;
	uint8_t s = (opcode >> 20) & 0x1;
	uint8_t a = (opcode >> 21) & 0x1;

	uint32_t value1 = ReadRegisterArm(rm);
	uint32_t value2 = ReadRegisterArm(rs);
	uint32_t value3 = ReadRegisterArm(rn);

	uint32_t result = 0;

	a ? result = value1 * value2 + value3 : result = value1 * value2;

	if (s)
	{
		//set condition code
		set_zer(result == 0);
		calc_neg(result);
	}

	SetRegisterArm(rd, result);
}

void Arm::MultiplyLong()
{
	uint8_t rm = opcode & 0xF;
	uint8_t rs = (opcode >> 8) & 0xF;
	uint8_t rdlo = (opcode >> 12) & 0xF;
	uint8_t rdhi = (opcode >> 16) & 0xF;
	uint8_t s = (opcode >> 20) & 0x1;
	uint8_t a = (opcode >> 21) & 0x1;
	uint8_t u = (opcode >> 22) & 0x1;


	if (!u)
	{
		u32 value1 = ReadRegisterArm(rm);
		u32 value2 = ReadRegisterArm(rs);
		u64 RDHI = ReadRegisterArm(rdhi);
		u64 RDLO = ReadRegisterArm(rdlo);
		u64 value3 = (RDHI << 32) | RDLO;
		u64 result = 0;

		a ? result = (u64)value1 * (u64)value2 + value3 : result = (u64)value1 * (u64)value2;

		if (s)
		{
			set_zer(result == 0);
			set_neg(result & 0x8000000000000000);
		}

		SetRegisterArm(rdlo, result & 0xFFFFFFFF);
		SetRegisterArm(rdhi, (result >> 32) & 0xFFFFFFFF);
	}
	else
	{
		i32 value1 = ReadRegisterArm(rm);
		i32 value2 = ReadRegisterArm(rs);
		i64 RDHI = ReadRegisterArm(rdhi);
		i64 RDLO = ReadRegisterArm(rdlo);
		i64 value3 = (RDHI << 32) | RDLO;
		i64 result = 0;

		a ? result = (i64)value1 * (i64)value2 + value3 : result = (i64)value1 * (i64)value2;

		if (s)
		{
			set_zer(result == 0);
			set_neg(result & 0x8000000000000000);
		}

		SetRegisterArm(rdlo, result & 0xFFFFFFFF);
		SetRegisterArm(rdhi, (result >> 32) & 0xFFFFFFFF);
	}
}

void Arm::SingleDataSwap()
{
	uint8_t rm = opcode & 0xF;
	uint8_t rd = (opcode >> 12) & 0xF;
	uint8_t rn = (opcode >> 16) & 0xF;
	uint8_t b = (opcode >> 22) & 0x1;

	uint32_t addr = ReadRegisterArm(rn);
	uint32_t value = 0;
	uint32_t reg_value = ReadRegisterArm(rm);

	if (!b)
	{
		value = RotateMisallignedWord(addr);
		CpuWrite32(addr, reg_value);
	}
	else
	{
		value = CpuRead(addr);
		CpuWrite(addr, reg_value);
	}

	SetRegisterArm(rd, value);
}

void Arm::BranchAndExchange()
{
	//BX
	u8 rn = opcode & 0xF;
	u32 value = ReadRegisterArm(rn);
	set_sta(value & 0x1);
	sta_res() ? SetRegisterThumb(15, value) : SetRegisterArm(15, value);
}

void Arm::HalfwordTransferReg()
{
	uint8_t rm = opcode & 0xF;
	uint8_t h = (opcode >> 5) & 0x1;
	uint8_t s = (opcode >> 6) & 0x1;
	uint8_t rd = (opcode >> 12) & 0xF;
	uint8_t rn = (opcode >> 16) & 0xF;
	uint8_t l = (opcode >> 20) & 0x1;
	uint8_t w = (opcode >> 21) & 0x1;
	uint8_t u = (opcode >> 23) & 0x1;
	uint8_t p = (opcode >> 24) & 0x1;

	uint32_t value = 0;

	u32 final_address = ReadRegisterArm(rn);

	if (!p)
	{
		DoLoadHalfword(l, h, s, rd, final_address);
		u ? final_address += ReadRegisterArm(rm) : final_address -= ReadRegisterArm(rm);
		w = 1; //always done in post indexing
		if (w)
		{
			if (l && (rn != rd)) SetRegisterArm(rn, final_address);
			else if (!l) SetRegisterArm(rn, final_address);
		}
	}
	else
	{
		u ? final_address += ReadRegisterArm(rm) : final_address -= ReadRegisterArm(rm);
		DoLoadHalfword(l, h, s, rd, final_address);
		if (w)
		{
			if (l && (rn != rd)) SetRegisterArm(rn, final_address);
			else if (!l) SetRegisterArm(rn, final_address);
		}
	}
}

void Arm::HalfwordTransferImm()
{
	uint8_t offset1 = opcode & 0xF;
	uint8_t h = (opcode >> 5) & 0x1;
	uint8_t s = (opcode >> 6) & 0x1;
	uint8_t offset2 = (opcode >> 8) & 0xF;
	uint8_t rd = (opcode >> 12) & 0xF;
	uint8_t rn = (opcode >> 16) & 0xF;
	uint8_t l = (opcode >> 20) & 0x1;
	uint8_t w = (opcode >> 21) & 0x1;
	uint8_t u = (opcode >> 23) & 0x1;
	uint8_t p = (opcode >> 24) & 0x1;

	uint8_t finalOffset = (offset2 << 4) | offset1;

	uint32_t value = 0;

	u32 final_address = ReadRegisterArm(rn);

	if (!p)
	{
		DoLoadHalfword(l, h, s, rd, final_address);
		u ? final_address += finalOffset : final_address -= -finalOffset;
		w = 1; //always done in post indexing
		if (w)
		{
			if (l && (rn != rd)) SetRegisterArm(rn, final_address);
			else if (!l) SetRegisterArm(rn, final_address);
		}
	}
	else
	{
		u ? final_address += finalOffset : final_address -= finalOffset;
		DoLoadHalfword(l, h, s, rd, final_address);
		if (w)
		{
			if (l && (rn != rd)) SetRegisterArm(rn, final_address);
			else if (!l) SetRegisterArm(rn, final_address);
		}
	}
}

void Arm::SingleDataTransfer()
{
	//Getting the relevant data
	u32 offset = opcode & 0xFFF;
	u8 src_register = (opcode >> 12) & 0xF;
	u8 base_register = (opcode >> 16) & 0xF;

	//Flags
	u8 load = (opcode >> 20) & 0x1;
	u8 write_back = (opcode >> 21) & 0x1;
	u8 byte = (opcode >> 22) & 0x1;
	u8 up_down = (opcode >> 23) & 0x1;
	u8 pre_post_indexing = (opcode >> 24) & 0x1;
	u32 immediate = (opcode >> 25) & 0x1;

	u32 operand1 = ReadRegisterArm(base_register);
	u32 operand2 = ReadRegisterArm(src_register);

	//Calculation Variables
	u32 carry_out = 0;
	u32 final_offset = 0;
	u32 final_address = 0;

	if (immediate) {
		immediate = 0; //setting it to be 0 so it goes into the right if-else loop
		DoShifting(immediate, offset, final_offset, carry_out);
	}
	else
	{
		final_offset = offset;
	}

	if (pre_post_indexing)
	{
		// pre indexing so add offset before transfer
		up_down ? final_address = operand1 + final_offset : final_address = operand1 - final_offset;

		DoLoadFlag(load, byte, final_address, src_register);
	}
	else
	{
		//post indexing so add offset after transfer

		write_back = 1;

		final_address = ReadRegisterArm(base_register);

		DoLoadFlag(load, byte, final_address, src_register);

		up_down ? final_address = operand1 + final_offset : final_address = operand1 - final_offset;
	}

	if (write_back)
	{
		if (load && (base_register != src_register)) SetRegisterArm(base_register, final_address);
		else if (!load) SetRegisterArm(base_register, final_address);
	}
}

void Arm::Undefined()
{
	//sets undefined mode
	cpsr &= 0xFFFFFFE0;
	cpsr |= 0x0000001B;

	printf("GBA does not use Undefined\n");
}

void Arm::BlockDataTransfer()
{
	u8 Rn = (opcode >> 16) & 0xF;
	u8	L = (opcode >> 20) & 0x1;
	u8 	write = (opcode >> 21) & 0x1;
	u8 	force_usr_mode = (opcode >> 22) & 0x1;
	u8 	U = (opcode >> 23) & 0x1;
	u8 	P = (opcode >> 24) & 0x1;
	u16 Rlist = opcode & 0xFFFF;

	//final_address is the address at the end of the operation which will be written back
	//address will be used in the instruction to read/write and will be incremented
	u8  list_reg_pos = 0;
	u32 address = ReadRegisterArm(Rn, force_usr_mode);
	u32 old_address = address;
	u32 final_address = address;

	if (Rlist != 0)
	{
		if (!U)
		{
			//DECREMENT
			address -= countSetBits(Rlist) * 4;
			if (!P) address += 4;
			final_address -= countSetBits(Rlist) * 4;
		}
		else
		{
			//INCREMENT
			if (P) address += 4;
			final_address += countSetBits(Rlist) * 4;
		}
	}
	else
	{
		if (!U)
		{
			//DECREMENT
			address -= 0x40;
			if (!P) address += 4;
			final_address -= 0x40;
		}
		else
		{
			//INCREMENT
			if (P) address += 4;
			final_address += 0x40;
		}
	}

	for (u8 shifter = 0; shifter < 16; shifter++)
	{
		if ((Rlist >> shifter) & 1)
		{
			list_reg_pos++;

			if (shifter == Rn)
			{
				if (!L)
				{
					//If rn is in the list, and store, check if it's the first register. If it is write new base if not write old base
					list_reg_pos == 1 ? CpuWrite32(address, old_address) : CpuWrite32(address, final_address);
				}
				else
				{
					//no writeback if rn is in the list and it's a load operation so write gets set to 0
					write = 0;
					SetRegisterArm(shifter, CpuRead32(address & ~3));
				}
			}
			else
			{
				u32 value = ReadRegisterArm(shifter, force_usr_mode) + (shifter == 15 ? 4 : 0);
				L ? SetRegisterArm(shifter, CpuRead32(address & ~3), force_usr_mode) : CpuWrite32(address, value);
			}

			if (force_usr_mode & L & (Rlist & 0x8000)) cpsr = ReadRegisterFlag();

			address += 4;
		}
	}

	if (Rlist == 0)
	{
		L ? SetRegisterArm(15, CpuRead32(address & ~3)) : CpuWrite32(address, ReadRegisterArm(15) + 4);
	}

	if (write) SetRegisterArm(Rn, final_address, force_usr_mode);
}

void Arm::Branch()
{
	int32_t offset = (opcode & 0x00FFFFFF) << 8;
	offset >>= 8;
	offset <<= 2;
	uint8_t l = (opcode >> 24) & 0x1;
	//if Link Bit is set Old PC value is stored in LR and adjusted for prefetch
	if (l) SetRegisterArm(14, ReadRegisterArm(15) - 4);
	SetRegisterArm(15, ReadRegisterArm(15) + offset);
}

void Arm::CoprocessorDataTransfer()
{
	printf("\nERROR! GBA does not use co processor instructions -> OPCODE : 0x%X\n", opcode);
}

void Arm::CoprocessorDataOperation()
{
	printf("\nERROR! GBA does not use co processor instructions -> OPCODE : 0x%X\n", opcode);
}

void Arm::CoprocessorRegisterTransfer()
{
	printf("\nERROR! GBA does not use co processor instructions -> OPCODE : 0x%X\n", opcode);
}

/*-----------------------THUMB OPCODES---------------------------------------------------------------*/
void Arm::MoveShiftedRegister()
{
	u32 op = (opcode >> 11) & 0x3;
	u8 RD = opcode & 0x7;
	u8 RS = (opcode >> 3) & 0x7;
	u32 offset = (opcode >> 6) & 0x1F;
	u32 result = 0, carry_out = 0;
	u32 operand1 = ReadRegisterThumb(RS);
	BarrelShifterImmediate(operand1, op, offset, result, carry_out);
	calc_neg(result);
	set_zer(result == 0);
	set_car(carry_out);
	SetRegisterThumb(RD, result);
}
void Arm::AddSubtract()
{
	u8 op = (opcode >> 9) & 0x1;
	u8 RD = opcode & 0x7;
	u8 RS = (opcode >> 3) & 0x7;
	u8 offset3 = (opcode >> 6) & 0x7;
	bool immediate = (opcode >> 10) & 0x1;
	u32 operand1 = ReadRegisterThumb(RS);
	u32 operand2 = ReadRegisterThumb(offset3);
	u32 result = 0;
	switch (op)
	{
	case 0x00:
	{
		if (immediate) {
			result = operand1 + offset3;
			operand2 = offset3;
		}
		else {
			result = operand1 + operand2;
		}

		calc_carry(operand1, operand2);
		calc_overflow(operand1, operand2, result);

		break;
	}
	case 0x01:
	{
		if (immediate)
		{
			result = operand1 - offset3;
			set_car(operand1 >= offset3);
			calc_overflow(operand1, ~offset3, result);
		}
		else
		{
			result = operand1 - operand2;
			set_car(operand1 >= operand2);
			calc_overflow(operand1, ~operand2, result);
		}
		break;
	}
	}
	set_zer(result == 0);
	calc_neg(result);
	SetRegisterThumb(RD, result);
}

void Arm::MovCompAddSubImm()
{
	uint8_t offset = opcode & 0xFF;
	uint8_t rd = (opcode >> 8) & 0x7;
	uint8_t op = (opcode >> 11) & 0x3;

	uint32_t result = 0;

	switch (op)
	{
	case 0x00:
	{
		//MOV 8bit Imm
		result = offset;
		calc_neg(result);
		set_zer(result == 0);
		SetRegisterThumb(rd, result);
		break;
	}
	case 0x01:
	{
		//CMP 8bit imm
		result = ReadRegisterThumb(rd) - offset;
		calc_overflow(ReadRegisterThumb(rd), ~offset, result);
		set_car(ReadRegisterThumb(rd) >= offset);
		calc_neg(result);
		set_zer(result == 0);
		break;
	}
	case 0x02:
	{
		// ADD 8bit imm
		result = ReadRegisterThumb(rd) + offset;
		calc_overflow(ReadRegisterThumb(rd), offset, result);
		calc_carry(ReadRegisterThumb(rd), offset);
		calc_neg(result);
		set_zer(result == 0);
		SetRegisterThumb(rd, result);
		break;
	}
	case 0x03:
	{
		// Sub 8bit imm
		result = ReadRegisterThumb(rd) - offset;
		calc_overflow(ReadRegisterThumb(rd), ~offset, result);
		set_car(ReadRegisterThumb(rd) >= offset);
		calc_neg(result);
		set_zer(result == 0);
		SetRegisterThumb(rd, result);
		break;
	}
	}
}

void Arm::AluOperations()
{
	u8 src_reg = opcode & 0x7;
	u8 base_reg = (opcode >> 3) & 0x7;
	u8 op = (opcode >> 6) & 0xf;

	u32 operand1 = ReadRegisterThumb(src_reg);
	u32 operand2 = ReadRegisterThumb(base_reg);

	u32 result = 0;

	bool t = true, p = true;

	switch (op)
	{
	case 0x00: { result = operand1 & operand2;  break; } //AND
	case 0x01: { result = operand1 ^ operand2;  break; } //EOR
	case 0x02: {
		//LSL
		u32 shift_type = 0x00;
		u32 carry_out = 0;
		BarrelShifter(operand1, shift_type, operand2, result, carry_out);
		if (operand2) set_car(carry_out);
		break;
	}
	case 0x03: {
		//LSR
		u32 shift_type = 0x01;
		u32 carry_out = 0;
		BarrelShifter(operand1, shift_type, operand2, result, carry_out);
		if (operand2) set_car(carry_out);
		break;
	}
	case 0x04:
	{
		//ASR
		u32 shift_type = 0x02;
		u32 carry_out = 0;
		BarrelShifter(operand1, shift_type, operand2, result, carry_out);
		if (operand2) set_car(carry_out);
		break;
	}
	case 0x05:
	{
		//ADC
		result = operand1 + operand2 + car_res();
		calc_overflow(operand1, operand2 + car_res(), result);
		set_car((u64)operand1 + (u64)operand2 + (u64)car_res() > 0xFFFFFFFF);
		//calc_carry(operand1, operand2 + car_res());
		break;
	}
	case 0x06:
	{
		//SBC
		result = operand1 - operand2 - !car_res();
		calc_overflow(operand1, ~(operand2 + !car_res()), result);
		set_car(operand1 >= operand2);
		break;
	}
	case 0x07: {
		//ROR
		u32 shift_type = 0x03;
		u32 carry_out = 0;
		BarrelShifter(operand1, shift_type, operand2, result, carry_out);
		if (operand2) set_car(carry_out);
		break;
	}
	case 0x08: { result = operand1 & operand2; t = false;  break; } //TST
	case 0x09: {
		//NEG
		result = 0 - operand2;
		set_car(operand2 == 0);
		calc_overflow(0, ~operand2, result);
		break;
	}
	case 0x0A:
	{
		//CMP
		result = operand1 - operand2;
		calc_overflow(operand1, ~operand2, result);
		set_car(operand1 >= operand2);
		t = false;
		break;
	}
	case 0x0B:
	{
		//CMN
		result = operand1 + operand2;
		calc_overflow(operand1, operand2, result);
		calc_carry(operand1, operand2);
		t = false;
		break;
	}
	case 0x0C: { result = operand1 | operand2;  break; } //ORR
	case 0x0D: { result = operand1 * operand2;  break; } //MOV
	case 0x0E: { result = operand1 & (~operand2); break; } //BIC
	case 0x0F: { result = ~operand2; break; } //MVN
	}

	//set common flags
	if (p)
	{
		calc_neg(result);
		set_zer(result == 0);
	}

	if (t) SetRegisterThumb(src_reg, result);
}

void Arm::BranchExchange()
{
	u8 rd = (opcode >> 0) & 0x7;
	u8 rs = (opcode >> 3) & 0x7;
	u8 h2 = (opcode >> 6) & 0x1;
	u8 h1 = (opcode >> 7) & 0x1;
	u8 op = (opcode >> 8) & 0x3;

	h1 ? rd += 8 : rd;
	h2 ? rs += 8 : rs;

	u32 operand1 = ReadRegisterArm(rd);
	u32 operand2 = ReadRegisterArm(rs);

	u32 result = 0;

	switch (op)
	{
	case 0x00:
	{
		//ADD
		result = operand1 + operand2;
		SetRegisterArm(rd, result);
		break;
	}
	case 0x01:
	{
		//CMP
		result = operand1 - operand2;
		calc_overflow(operand1, ~operand2, result);
		set_car(operand1 >= operand2);
		calc_neg(result);
		set_zer(result == 0);
		break;
	}
	case 0x02:
	{
		//MOV
		result = operand2;
		SetRegisterArm(rd, result);
		break;
	}
	case 0x03:
	{
		//BX
		set_sta(operand2 & 0x1);
		sta_res() ? SetRegisterThumb(15, operand2) : SetRegisterArm(15, operand2);
		break;
	}
	}
}

void Arm::PcRelativeLoad()
{
	uint16_t word8 = opcode & 0xFF;
	uint8_t rd = (opcode >> 8) & 0x7;
	uint32_t addr = system[15] + (word8 << 2);
	addr &= 0xFFFFFFFC;
	u32 value = CpuRead32(addr);
	SetRegisterThumb(rd, value);
}

void Arm::SpRelativeLoadStore()
{
	uint16_t word8 = opcode & 0xFF;
	uint8_t rd = (opcode >> 8) & 0x7;
	uint8_t l = (opcode >> 11) & 0x1;
	//PC bit 1 set 0 and bit 1:0 of word8 set to 0 to ensure word aligned
	uint32_t addr = ReadRegisterThumb(13) + (word8 << 2);
	if (l)
	{
		u32 value = RotateMisallignedWord(addr);
		SetRegisterThumb(rd, value);
	}
	else
	{
		CpuWrite32(addr, ReadRegisterThumb(rd));
	}
}

void Arm::PushPopRegisters()
{
	uint8_t rlist = opcode & 0xFF;
	uint8_t r = (opcode >> 8) & 0x1;
	uint8_t l = (opcode >> 11) & 0x1;

	if (l)
	{
		for (int shifter = 0; shifter < 8; shifter++)
		{
			//Post increment addressing
			if ((rlist >> shifter) & 0x1)
			{
				SetRegisterThumb(shifter, CpuRead32(ReadRegisterThumb(13)));//Load first
				SetRegisterThumb(13, ReadRegisterThumb(13) + 4);// Then increment
			}
		}

		if (r)
		{
			//Load PC
			SetRegisterThumb(15, (CpuRead32(ReadRegisterThumb(13))) & ~1);
			SetRegisterThumb(13, ReadRegisterThumb(13) + 4);
		}
	}
	else
	{
		if (r)
		{
			//Store LR
			SetRegisterThumb(13, ReadRegisterThumb(13) - 4);
			CpuWrite32(ReadRegisterThumb(13), ReadRegisterThumb(14));
		}

		for (int shifter = 8; shifter > 0; shifter--)
		{
			if ((rlist >> (shifter - 1)) & 0x1)
			{
				//Pre decrement addressing 
				SetRegisterThumb(13, ReadRegisterThumb(13) - 4); //Decrement SP
				CpuWrite32(ReadRegisterThumb(13), ReadRegisterThumb(shifter - 1));//Store register
			}
		}
	}
}

void Arm::AddOffsetToStackPointer()
{
	uint16_t sword7 = opcode & 0x7F;
	uint8_t s = (opcode >> 7) & 0x1;
	sword7 <<= 2;
	s ? SetRegisterThumb(13, ReadRegisterThumb(13) - sword7) : SetRegisterThumb(13, ReadRegisterThumb(13) + sword7);
}

void Arm::MultipleLoadStore()
{
	uint8_t rlist = opcode & 0xFF;
	uint8_t rb = (opcode >> 8) & 0x7;
	uint8_t l = (opcode >> 11) & 0x1;
	if (rlist != 0)
	{
		if (l)
		{
			for (int shifter = 0; shifter < 8; shifter++)
			{
				//Post increment addressing
				if ((rlist >> shifter) & 0x1)
				{
					SetRegisterThumb(shifter, CpuRead32(ReadRegisterThumb(rb)));//Load first
					SetRegisterThumb(rb, ReadRegisterThumb(rb) + 4);// Then increment
				}
			}
		}
		else
		{
			u32 old_base = ReadRegisterThumb(rb);
			int reg_count = 0;
			for (int shifter = 0; shifter < 8; shifter++)
			{
				if ((rlist >> shifter) & 0x1)
				{
					if (rb == shifter)
					{
						if (reg_count == 0)
							CpuWrite32(ReadRegisterThumb(rb), old_base);
						else
							CpuWrite32(ReadRegisterThumb(rb), old_base + countSetBits(rlist) * 4);
					}
					else
					{
						CpuWrite32(ReadRegisterThumb(rb), ReadRegisterThumb(shifter));//Store register
					}
					SetRegisterThumb(rb, ReadRegisterThumb(rb) + 4); //Decrement SP
					reg_count++;
				}
			}
		}
	}
	else
	{
		if (l)
		{
			SetRegisterThumb(15, CpuRead32(ReadRegisterThumb(rb)));
		}
		else
		{
			CpuWrite32(ReadRegisterThumb(rb), ReadRegisterThumb(15) + 2);
		}
		SetRegisterThumb(rb, ReadRegisterThumb(rb) + 0x40);
	}
}

void Arm::LoadAddress()
{
	uint16_t word8 = opcode & 0xFF;
	uint8_t rd = (opcode >> 8) & 0x7;
	uint8_t sp = (opcode >> 11) & 0x1;
	uint16_t final_word = ((word8) << 2) & 0xFFFFFFFC;
	sp ? SetRegisterThumb(rd, final_word + ReadRegisterThumb(13)) : SetRegisterThumb(rd, final_word + (ReadRegisterThumb(15) & ~2));
}

void Arm::LoadStoreRegOffset()
{
	uint8_t rd = opcode & 0x7;
	uint8_t rb = (opcode >> 3) & 0x7;
	uint8_t ro = (opcode >> 6) & 0x7;
	uint8_t b = (opcode >> 10) & 0x1;
	uint8_t l = (opcode >> 11) & 0x1;

	uint32_t addr = ReadRegisterThumb(rb) + ReadRegisterThumb(ro);
	uint32_t value = 0;

	if (l)
	{
		if (b)
		{
			value = CpuRead(addr);
		}
		else
		{
			//const auto rotate = (addr & 3) * 8;
			//addr &= ~3;
			//value = CpuRead32(addr);
			//if (rotate) value = (value << (32 - rotate)) | (value >> rotate);
			value = RotateMisallignedWord(addr);
		}

		SetRegisterThumb(rd, value);
	}
	else
	{
		if (b)
		{
			value = ReadRegisterThumb(rd) & 0xFF;
			CpuWrite(addr, value);
		}
		else
		{
			value = ReadRegisterThumb(rd);
			CpuWrite32(addr & ~3, value);
		}
	}
}

void Arm::LoadStoreSignExtend()
{
	uint8_t rd = opcode & 0x7;
	uint8_t rb = (opcode >> 3) & 0x7;
	uint8_t ro = (opcode >> 6) & 0x7;
	uint8_t s = (opcode >> 10) & 0x1;
	uint8_t h = (opcode >> 11) & 0x1;

	uint32_t addr = ReadRegisterThumb(rb) + ReadRegisterThumb(ro);
	uint32_t value = 0;

	if ((s == 0) && (h == 0))
	{
		//Store halfword
		value = ReadRegisterThumb(rd);
		CpuWrite16(addr & ~1, value & 0xFFFF);
	}
	else
	{
		if (h)
		{
			if (!s)
			{
				value = RotateMisallignedHalfWord(addr);
			}
			else
			{
				if (addr & 1)
				{
					//signed misaligned addresses
					value = CpuRead(addr);
					int32_t signed_value = value << 24;
					signed_value >>= 24;
					value = signed_value;
				}
				else
				{
					//regular sign extension
					value = CpuRead16(addr);
					int32_t signed_value = value << 16;
					signed_value >>= 16;
					value = signed_value;
				}
			}
		}
		else
		{
			value = CpuRead(addr);
			if (s)
			{
				int32_t signed_value = value << 24;
				signed_value >>= 24;
				value = signed_value;
			}
		}
		SetRegisterThumb(rd, value);
	}
}

void Arm::LoadStoreImmOffset()
{
	uint8_t rd = opcode & 0x7;
	uint8_t rb = (opcode >> 3) & 0x7;
	uint8_t offset5 = (opcode >> 6) & 0x1F;
	uint8_t l = (opcode >> 11) & 0x1;
	uint8_t b = (opcode >> 12) & 0x1;

	uint32_t addr = ReadRegisterThumb(rb) + offset5;
	uint32_t value = 0;

	if (l)
	{
		if (b)
		{
			value = CpuRead(addr);
		}
		else
		{
			addr = ReadRegisterThumb(rb) + (offset5 << 2);
			value = RotateMisallignedWord(addr);
		}

		SetRegisterThumb(rd, value);
	}
	else
	{
		if (b)
		{
			value = ReadRegisterThumb(rd) & 0xFF;
			CpuWrite(addr, value);
		}
		else
		{
			value = ReadRegisterThumb(rd);
			addr = (ReadRegisterThumb(rb) + (offset5 << 2));
			CpuWrite32(addr & ~3, value);
		}
	}
}

void Arm::LoadStoreHalfword()
{
	uint8_t rd = opcode & 0x7;
	uint8_t rb = (opcode >> 3) & 0x7;
	uint8_t offset5 = (opcode >> 6) & 0x1F;
	uint8_t l = (opcode >> 11) & 0x1;

	uint32_t addr = ReadRegisterThumb(rb) + (offset5 << 1);
	uint32_t value = CpuRead16(addr) & 0xFFFF;

	if (l)
	{
		value = RotateMisallignedHalfWord(addr);
		SetRegisterThumb(rd, value);
	}
	else
	{
		CpuWrite16(addr, ReadRegisterThumb(rd));
	}
}

void Arm::ConditionalBranch()
{
	int16_t offset = opcode & 0xFF;
	uint8_t cond = (opcode >> 8) & 0xF;
	offset <<= 8;
	offset >>= 8;
	offset <<= 1;
	if (ConditionField(cond))
	{
		SetRegisterThumb(15, ReadRegisterThumb(15) + offset);
	}
}

void Arm::UnconditionalBranch()
{
	uint16_t ofset11 = opcode & 0x7FF;
	SetRegisterThumb(15, ReadRegisterThumb(15) + ((int16_t)(ofset11 << 5) >> 4));
}

void Arm::LongBranchWithLink()
{
	uint16_t offset = opcode & 0x7FF;
	uint8_t h = (opcode >> 11) & 0x1;
	//pc is already 4 bytes ahead, need to account for the offset
	if (!h)
	{
		int32_t target_address = offset << 21;
		target_address >>= 21;
		SetRegisterThumb(14, ReadRegisterThumb(15) + (target_address << 12));
	}
	else
	{
		uint32_t temp = system[15] - 2;
		SetRegisterThumb(15, ReadRegisterThumb(14) + (offset << 1));
		SetRegisterThumb(14, temp | 1);
	}
}

/*-----------------------------INTERRUPTS----------------------------------------------------------*/

void Arm::SoftwareInterrupt()
{
	//Store PC in LR
	supervisor[1] = sta_res() ? system[15] - 2 : system[15] - 4;
	//Store cpsr in spsr_svc
	spsr[2] = cpsr;
	//Enter Supervisor mode
	cpsr &= ~0xFF; //MIGHT BE AN ERROR
	cpsr |= 0x13;
	//Set T bit to 0 to enter ARM mode
	set_sta(0);
	//Set IRQ to 0 ->sets disable IRQ bit
	set_irq(1);
	//Set FIQ to 0 -> sets disable FIQ bit
	//set_fiq(1);
	//PC = exception vector
	SetRegisterArm(15, 0x08);

	//busPtr->latch = CpuRead32(0x190);
}

void Arm::SkipBios()
{
	system[0] = 0x08000000;
	system[1] = 0x000000EA;
	system[15] = 0x08000000;
	cpsr = 0x6000001F;
}

void Arm::InterruptRequest()
{
	//printf("I am in IRQ: program_counter: %x\n", system[15]);
	//Store PC in LR
	irq[1] = sta_res() ? system[15] : system[15] - 4;
	//Store cpsr in spsr_irq
	spsr[1] = cpsr;
	//Enter irq mode
	cpsr &= ~0x1F;
	cpsr |= 0x12;
	//Set T bit to 0
	set_sta(0);
	//Set IRQ to 0 only in priviliged mode
	set_irq(1);
	//PC = exception vector
	SetRegisterArm(15, 0x18);
}

/*----------------------CPU RELATED FUNCTIONS--------------------------------------------------------*/

u32 Arm::fetch()
{
	if (!sta_res())
	{
		u8 b1 = CpuRead(system[15] + 0);
		u8 b2 = CpuRead(system[15] + 1);
		u8 b3 = CpuRead(system[15] + 2);
		u8 b4 = CpuRead(system[15] + 3);
		return (b4 << 24) | (b3 << 16) | (b2 << 8) | b1;
	}
	else
	{
		u8 b1 = CpuRead(system[15] + 0);
		u8 b2 = CpuRead(system[15] + 1);
		return (b2 << 8) | b1;
	}
}

void Arm::decodeArm()
{
	u8 cond = (opcode >> 28) & 0xf;
	if (ConditionField(cond))
	{
		if ((opcode & 0x0F000000) == 0x0F000000) { InstructionAddress = &Arm::SoftwareInterrupt; }
		else if ((opcode & 0x0E000000) == 0x0C000000) { InstructionAddress = &Arm::CoprocessorDataTransfer; }
		else if ((opcode & 0x0F000010) == 0x0E000000) { InstructionAddress = &Arm::CoprocessorDataOperation; }
		else if ((opcode & 0x0F000010) == 0x0E000010) { InstructionAddress = &Arm::CoprocessorRegisterTransfer; }
		else if ((opcode & 0x0E000000) == 0x0A000000) { InstructionAddress = &Arm::Branch; }
		else if ((opcode & 0x0E000000) == 0x08000000) { InstructionAddress = &Arm::BlockDataTransfer; }
		else if ((opcode & 0x0E000010) == 0x08000010) { InstructionAddress = &Arm::Undefined; }
		else if ((opcode & 0x0C000000) == 0x04000000) { InstructionAddress = &Arm::SingleDataTransfer; }
		else if ((opcode & 0x0FFFFFF0) == 0x012FFF10) { InstructionAddress = &Arm::BranchAndExchange; }
		else if ((opcode & 0x0FB00FF0) == 0x01000090) { InstructionAddress = &Arm::SingleDataSwap; }
		else if ((opcode & 0x0F8000F0) == 0x00800090) { InstructionAddress = &Arm::MultiplyLong; }
		else if ((opcode & 0x0FC000F0) == 0x00000090) { InstructionAddress = &Arm::Multiply; }
		else if ((opcode & 0x0E400F90) == 0x00000090) { InstructionAddress = &Arm::HalfwordTransferReg; }
		else if ((opcode & 0x0E400090) == 0x00400090) { InstructionAddress = &Arm::HalfwordTransferImm; }
		else if ((opcode & 0x0FBF0FFF) == 0x010F0000) { InstructionAddress = &Arm::MRS; }
		else if ((opcode & 0x0FB00000) == 0x03200000) { InstructionAddress = &Arm::MSR; }
		else if ((opcode & 0x0FB000F0) == 0x01200000) { InstructionAddress = &Arm::MSR; }
		else if ((opcode & 0x0C000000) == 0x00000000) { InstructionAddress = &Arm::DataProcessing; }
	}
}

void Arm::decodeThumb()
{
	if ((opcode & 0xF000) == 0xF000) { InstructionAddress = &Arm::LongBranchWithLink; }
	else if ((opcode & 0xF800) == 0xE000) { InstructionAddress = &Arm::UnconditionalBranch; }
	else if ((opcode & 0xFF00) == 0xDF00) { InstructionAddress = &Arm::SoftwareInterrupt; }
	else if ((opcode & 0xF000) == 0xD000) { InstructionAddress = &Arm::ConditionalBranch; }
	else if ((opcode & 0xF000) == 0xC000) { InstructionAddress = &Arm::MultipleLoadStore; }
	else if ((opcode & 0xF600) == 0xB400) { InstructionAddress = &Arm::PushPopRegisters; }
	else if ((opcode & 0xFF00) == 0xB000) { InstructionAddress = &Arm::AddOffsetToStackPointer; }
	else if ((opcode & 0xF000) == 0xA000) { InstructionAddress = &Arm::LoadAddress; }
	else if ((opcode & 0xF000) == 0x9000) { InstructionAddress = &Arm::SpRelativeLoadStore; }
	else if ((opcode & 0xF000) == 0x8000) { InstructionAddress = &Arm::LoadStoreHalfword; }
	else if ((opcode & 0xE000) == 0x6000) { InstructionAddress = &Arm::LoadStoreImmOffset; }
	else if ((opcode & 0xF200) == 0x5200) { InstructionAddress = &Arm::LoadStoreSignExtend; }
	else if ((opcode & 0xF200) == 0x5000) { InstructionAddress = &Arm::LoadStoreRegOffset; }
	else if ((opcode & 0xF800) == 0x4800) { InstructionAddress = &Arm::PcRelativeLoad; }
	else if ((opcode & 0xFC00) == 0x4400) { InstructionAddress = &Arm::BranchExchange; }
	else if ((opcode & 0xFC00) == 0x4000) { InstructionAddress = &Arm::AluOperations; }
	else if ((opcode & 0xE000) == 0x2000) { InstructionAddress = &Arm::MovCompAddSubImm; }
	else if ((opcode & 0xF800) == 0x1800) { InstructionAddress = &Arm::AddSubtract; }
	else if ((opcode & 0xE000) == 0x0000) { InstructionAddress = &Arm::MoveShiftedRegister; }
}

void Arm::flushPipeline()
{
	pipeline[0] = fetch();
	sta_res() ? system[15] += 2 : system[15] += 4;
	pipeline[1] = fetch();
	sta_res() ? system[15] += 2 : system[15] += 4;
}

void Arm::resetCpu()
{
	opcode = 0x00;

	for (int i = 0; i < 16; i++)
	{
		system[i] = 0;
		if (i < 8)
			fiq[i] = 0;
		if (i < 6)
			spsr[i] = 0;
	}

	for (int i = 0; i < 2; i++)
	{
		supervisor[i] = 0;
		abort[i] = 0;
		irq[i] = 0;
		undefined[i] = 0;
		pipeline[i] = 0;
	}

	supervisor[0] = 0x03007FE0; //set svc sp
	irq[0] = 0x03007FA0; // set irq sp
	system[13] = 0x03007F00; //set system sp
	cpsr = 0x0000001F;

	InstructionAddress = NULL;
}

void Arm::runCpu(int cyclesToRunFor)
{
	int CurrentCycles = cyclesToRunFor;

	//GBA cpu always starts in Arm mode
	flushPipeline();

	while (CurrentCycles > 0)
	{
		stepCpu();
		CurrentCycles--;
	}
}

u32 Arm::stepCpu()
{
	opcode = pipeline[0];
	pipeline[0] = pipeline[1];
	piping = true;
	pipeline[1] = fetch();
	if (system[15] < 0x4000)
	{
		busPtr->latch = pipeline[1];
	}
	piping = false;
	InstructionAddress = &Arm::DoNothing;
	sta_res() ? decodeThumb() : decodeArm();
	//if (busPtr->c)
	//{
	//	printf("pc : 0x%002x Opcode : 0x%002x\n", system[15], opcode);
	//	for (int i = 0; i < 16; i++)
	//		printf(" R%d : 0x%002x", i, system[i]);
	//	printf("\n\n");
	//}
	/*MyFile << "opcode : 0x" << std::hex << opcode << std::endl;
	MyFile << "pc : 0x" << std::hex << system[15] << std::endl;
	MyFile << "r0 : 0x" << std::hex << system[0] 
		<< " " << "r1 : 0x" << std::hex << system[1]
		<< " " << "r2 : 0x" << std::hex << system[2]
		<< " " << "r3 : 0x" << std::hex << system[3]
		<< " " << "r4 : 0x" << std::hex << system[4]
		<< " " << "r5 : 0x" << std::hex << system[5]
		<< " " << "r6 : 0x" << std::hex << system[6]
		<< " " << "r7 : 0x" << std::hex << system[7]
		<< " " << "r8 : 0x" << std::hex << system[8]
		<< " " << "r9 : 0x" << std::hex << system[9]
		<< " " << "r10 : 0x" << std::hex << system[10]
		<< " " << "r11 : 0x" << std::hex << system[11]
		<< " " << "r12 : 0x" << std::hex << system[12]
		<< " " << "r13 : 0x" << std::hex << system[13]
		<< " " << "r14 : 0x" << std::hex << system[14];
	MyFile << "\n";*/
	if (opcode != 0x4770dbfc) (this->*InstructionAddress)();
//	printf("Opcode : 0x%002x R0 : 0x%002x\n", opcode, system[0]);
	sta_res() ? system[15] += 2 : system[15] += 4;
	return 0;
}

Arm::~Arm()
{
	//MyFile.close();
	busPtr = NULL;
}