#include "mem.hpp"

Mem::Mem()
{
	mem.resize(0xff);
}

void Mem::resize(int size)
{
	mem.resize(size);
}

void Mem::init(int reset_value)
{
	for (unsigned int i = 0; i < mem.size(); i++)
	{
		mem[i] = reset_value;
	}
}

void Mem::set(uint32_t addr, uint8_t data)
{
	mem[addr] = data;
}

void Mem::load(char const* filepath, int start, int bytes)
{
	std::vector<uint8_t> result(bytes);
	std::ifstream file(filepath, std::ios::in | std::ios::binary);
	if (file.is_open())
	{
		printf("File read done\n\n");
		file.read((char*)&result[0], bytes);
		for (int i = 0; i < bytes; i++)
		{
			mem[start + i] = result[i];
		}
	}
	else
	{
		printf("\nFile could not be opened\n");
	}
}

uint8_t Mem::operator[](uint32_t addr)const
{
	return mem[addr];
}

uint8_t& Mem::operator[](uint32_t addr)
{
	return mem[addr];
}

void Mem::contents()
{
	for (unsigned int i = 0; i < mem.size(); i++)
	{
		printf("%x ", mem[i]);
	}
}