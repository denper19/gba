#pragma once

#include <vector>
#include <fstream>
#include <cstdint>

class Mem
{
private:

public:
	std::vector<uint8_t> mem;

	Mem();
	void resize(int size);
	void init(int);
	void set(uint32_t addr, uint8_t data);
	uint8_t operator[](uint32_t)const;
	uint8_t& operator[](uint32_t);
	void load(char const*, int start, int bytes);
	void contents();
};
