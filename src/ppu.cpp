#include "ppu.hpp"
#include "bus.hpp"

int pixels_x[4][3] =
{
	{8,  16,  8},
	{16, 32,  8},
	{32, 32, 16},
	{64, 64, 32}
};

int pixels_y[4][3] =
{
	{8,  8,  16},
	{16, 8,  32},
	{32, 16, 32},
	{64, 32, 64}
};

u32 background_size_x[] = { 256, 512, 256, 512 };
u32 background_size_y[] = { 256, 256, 512, 512 };
u32 affine_table[] = { 128, 256, 512, 1024 };

Lcd::Lcd()
{
	SDL_Init(SDL_INIT_VIDEO);
	window = SDL_CreateWindow("", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 240 * 4, 160 * 4, 0);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR1555, SDL_TEXTUREACCESS_STATIC, width, height);
	background_buffer = new bg_pixel_data[width * height];
	sprite_buffer = new obj_pixel_data[width * height];
	busPtr = nullptr;
}

void Lcd::ConnectBusToPPU(Bus* ptr)
{
	busPtr = ptr;
}

void Lcd::LcdWrite8(u32 addr, u8 data)
{
	busPtr->BusWrite(addr, data);
}

u8 Lcd::LcdRead8(u32 addr)
{
	return busPtr->BusRead(addr);
}

void Lcd::LcdWrite16(u32 addr, u16 data)
{
	u8 byte1 = (data >> 0) & 0xFF;
	u8 byte2 = (data >> 8) & 0xFF;
	busPtr->BusWrite(addr + 0, byte1);
	busPtr->BusWrite(addr + 1, byte2);
}

u16 Lcd::LcdRead16(u32 addr)
{
	u8 byte1 = busPtr->BusRead(addr + 0);
	u8 byte2 = busPtr->BusRead(addr + 1);
	u32 value = (byte2 << 8) | (byte1);
	return value;
}

u32 Lcd::LcdRead32(u32 addr)
{
	u8 byte1 = busPtr->BusRead(addr + 0);
	u8 byte2 = busPtr->BusRead(addr + 1);
	u8 byte3 = busPtr->BusRead(addr + 2);
	u8 byte4 = busPtr->BusRead(addr + 3);
	u32 value = (byte4 << 24) | (byte3 << 16) | (byte2 << 8) | (byte1);
	return value;
}


template<typename T>
void Lcd::LcdReadOam(const u32 addr, T& value) {
	memcpy(&value, (u8*)busPtr->OAM.mem.data() + addr, sizeof(T));
}

template<typename T>
void Lcd::LcdReadVram(const u32 addr, T& value) {
	memcpy(&value, (u8*)busPtr->VRAM.mem.data() + addr, sizeof(T));
}

template<typename T>
void Lcd::LcdReadPal(const u32 addr, T& value) {
	memcpy(&value, (u8*)busPtr->COLOR.mem.data() + addr, sizeof(T));
}

u32 Lcd::ReadRegisters(u32 addr)
{
	return LcdRead16(addr);
}

void Lcd::WriteRegisters(u32 addr, u32 data)
{
	LcdWrite16(addr, data);
}

void Lcd::UpdateFramePerLine(u16 scanline)
{
	int VideoData = ReadRegisters(REG_DISPCNT);
	BGaffine = false;

	//Chooses the drawing order of the arrays
	bool win0_en = VideoData & 0x2000;
	bool win1_en = VideoData & 0x4000;
	bool objw_en = VideoData & 0x8000;

	//	if (!(win0_en || win1_en))
	//	{
	switch (VideoData & 0x7)
	{
	case 0:
		if (VideoData & 0x100) DrawBackground(0, scanline, 240, 0, 0);
		if (VideoData & 0x200) DrawBackground(0, scanline, 240, 0, 1);
		if (VideoData & 0x400) DrawBackground(0, scanline, 240, 0, 2);
		if (VideoData & 0x400) DrawBackground(0, scanline, 240, 0, 3);
		break;
	case 1:
		if (VideoData & 0x100) DrawBackground(0, scanline, 240, 0, 0);
		if (VideoData & 0x200) DrawBackground(0, scanline, 240, 0, 1);
		if (VideoData & 0x400) {
			BGaffine = true;
			DrawBackground(0, scanline, 240, 0, 2);
		}
		break;
	case 2:
		BGaffine = true;
		if (VideoData & 0x400) DrawBackground(0, scanline, 240, 0, 2);
		if (VideoData & 0x800) DrawBackground(0, scanline, 240, 0, 3);
		break;
	case 3:
		if (VideoData & 0x400) DrawBitmapMode3(scanline);
		break;
	case 4:
		if (VideoData & 0x400) DrawBitmapMode4(scanline);
		break;
	case 5:
		if (VideoData & 0x400) DrawBitmapMode5(scanline);
		break;
	}

	if (VideoData & 0x1000)
		DrawSprites(0, scanline, 240, 0);
	//}
	//else
	//{
	//	drawWindow(VideoData, scanline);
	//}

	HandleBackgroundSpritePriority(scanline);
}

void Lcd::stepLcd()
{
	if (ReadRegisters(REG_VCOUNT) < 160)
	{
		//draw line of frame every hblank for all visible scanlines
		if (Cycles_Per_Line == 241)
		{
			UpdateFramePerLine(ReadRegisters(REG_VCOUNT));
			//increment every scanline
			BG2aff.ref_x += BG2aff.dmx; // increment dx by dmx
			BG2aff.ref_y += BG2aff.dmy; // increment dy by dmy
			BG3aff.ref_x += BG3aff.dmx;
			BG3aff.ref_y += BG3aff.dmy;
		}
	}
	else if ((ReadRegisters(REG_VCOUNT) >= 160) && (ReadRegisters(REG_VCOUNT) < 228))
	{
		//VBLANK
		if ((ReadRegisters(REG_VCOUNT) == 160) && (Cycles_Per_Line == 1))
		{
			//Set Vblank flag
			busPtr->inVblank = true;
			WriteRegisters(REG_DISPSTAT, LcdRead16(REG_DISPSTAT) | 0x1);
			if (ReadRegisters(REG_DISPSTAT) & 0x8)
				busPtr->IOREG[REG_IF - 0x04000000] |= 1;

			//reset is performed every vblank on the internal registers
			BG2aff.ref_x = LcdRead32(REG_BG2X); // reset dx
			BG2aff.ref_y = LcdRead32(REG_BG2Y); // reset dy
			BG2aff.dmx = LcdRead32(0x4000022);
			BG2aff.dmy = LcdRead32(0x4000026);

			//reset is performed every vblank on the internal registers
			BG3aff.ref_x = LcdRead32(REG_BG3X); // reset dx
			BG3aff.ref_y = LcdRead32(REG_BG3Y); // reset dy
			BG3aff.dmx = LcdRead32(0x4000032);
			BG3aff.dmy = LcdRead32(0x4000036);
		}

		if ((ReadRegisters(REG_VCOUNT) == 227) && (Cycles_Per_Line == 1))
		{
			//Clear Vblank
			busPtr->inVblank = false;
			WriteRegisters(REG_DISPSTAT, LcdRead16(REG_DISPSTAT) & 0xFFFFFFFE);
		}
	}

	if ((ReadRegisters(REG_VCOUNT) == ((ReadRegisters(REG_DISPSTAT) >> 8) & 0xFF)) && (Cycles_Per_Line == 1))
	{
		WriteRegisters(REG_DISPSTAT, ReadRegisters(REG_DISPSTAT) | 4);
		if (ReadRegisters(REG_DISPSTAT) & 0x20)
			busPtr->IOREG[REG_IF - 0x04000000] |= 4;
	}
	else
	{
		WriteRegisters(REG_DISPSTAT, ReadRegisters(REG_DISPSTAT) & 0xFFFFB);
	}

	//set Hblank flag
	if (Cycles_Per_Line == 241)
	{
		busPtr->inHblank = true;
		WriteRegisters(REG_DISPSTAT, LcdRead16(REG_DISPSTAT) | 2);
		if (ReadRegisters(REG_DISPSTAT) & 0x10)
			busPtr->IOREG[REG_IF - 0x04000000] |= 2;
	}

	if (Cycles_Per_Line > 308)
	{
		//Reset cycles
		Cycles_Per_Line = 0;

		//Line is over so increment LY
		WriteRegisters(REG_VCOUNT, LcdRead16(REG_VCOUNT) + 1);

		//Clear Hblank flag at the end of the line
		busPtr->inHblank = false;
		u8 data = busPtr->IOREG[REG_DISPSTAT - 0x04000000];
		data &= ~2;
		busPtr->IOREG[REG_DISPSTAT - 0x04000000] = data;
	}

	if (ReadRegisters(REG_VCOUNT) >= 228)
	{
		//If scanlines go over 227, then reset LY = 0
		WriteRegisters(REG_VCOUNT, 0);
	}

	Cycles_Per_Line++;
}

void Lcd::DrawBitmapMode3(u16 scanline)
{
	/*
	  Vram :=
	  06000000-06013FFF  80 KBytes Frame 0 buffer (only 75K actually used)
	  06014000-06017FFF  16 KBytes OBJ Tiles
	*/

	//240 pixels per line, 2 bytes per pixel since it's 16bpp, so 480 bytes per scanline
	u8 priority = LcdRead16(2 * 2 + 0x4000008) & 0x3;
	for (int i = 0; i < 480; i += 2)
	{
		u16 PixelColorIndex = LcdRead16(0x06000000 + scanline * 480 + i);
		background_buffer[scanline * 240 + (i / 2)] = bg_pixel_data{ 2, priority , PixelColorIndex };
	}
}

void Lcd::DrawBitmapMode4(u16 scanline)
{
	u8 priority = LcdRead16(2 * 2 + 0x4000008) & 0x3;
	u32 page = ReadRegisters(REG_DISPCNT) & 0x10 ? 0x0600A000 : 0x06000000;
	for (int i = 0; i < 240; i++)
	{
		int PixelColorIndex = LcdRead8(page + scanline * 240 + i);
		u16 color = LcdRead16(0x05000000 + PixelColorIndex * 2);
		background_buffer[scanline * 240 + i] = bg_pixel_data{ 2, priority, color };
	}
}

void Lcd::DrawBitmapMode5(u16 scanline)
{
	/*
		Bitmap mode is 160 x 128 px and scanline goes up to 240 so the scanline has to be brought to [0, 127] range
	*/
	u8 priority = LcdRead16(2 * 2 + 0x4000008) & 0x3;
	u32 page = ReadRegisters(REG_DISPCNT) & 0x10 ? 0x0600A000 : 0x06000000;
	if (scanline < 128)
	{
		for (int i = 0; i < 480; i += 2)
		{
			if (i < 320)
			{
				u16 PixelColorIndex = LcdRead16(page + scanline * 320 + i);
				background_buffer[scanline * 240 + (i / 2)] = bg_pixel_data{ 2, priority, PixelColorIndex };
			}
		}
	}
}

void Lcd::DrawBackground(int posX, int currentY, int endX, int type, int mode)
{
	u16 background_data = ReadRegisters(0x04000008 + 2 * mode);
	BGinfo.bg_hofs = ReadRegisters(0x4000010 + 4 * mode) & 0xFFFF;
	BGinfo.bg_vofs = ReadRegisters(0x4000012 + 4 * mode) & 0xFFFF;

	BGinfo.priority = (background_data >> 0) & 0x3;
	BGinfo.char_base = (background_data >> 2) & 0x3;
	BGinfo.mosaic_en = (background_data >> 6) & 0x1;
	BGinfo.color_mode = (background_data >> 7) & 0x1;
	BGinfo.screen_base = (background_data >> 8) & 0x1F;
	BGinfo.affine_wrap = (background_data >> 13) & 0x1;
	BGinfo.tile_size = (background_data >> 14) & 0x3;
	BGinfo.bg_hofs %= (background_size_x[BGinfo.tile_size]);
	BGinfo.bg_vofs %= (background_size_y[BGinfo.tile_size]);

	BGinfo.mode = mode;

	if (!BGaffine)
	{
		BG_normalCalc(posX, endX, currentY);
	}
	else
	{
		BG_affineCalc(posX, endX, currentY);
	}
}

void Lcd::BG_affineCalc(int start, int end, int scanline)
{
	// make a local copy of the registers, they will be incremented per pixel but the calculations
	//will be reset after every scanline so there should be no effect on the original registers
	s32 refx = BGinfo.mode == 2 ? BG2aff.ref_x : BG3aff.ref_x;
	s32 refy = BGinfo.mode == 2 ? BG2aff.ref_y : BG3aff.ref_y;

	//get matrix coeff
	s16 dx = BGinfo.mode == 2 ? LcdRead16(0x4000020) : LcdRead16(0x4000030);
	s16 dy = BGinfo.mode == 2 ? LcdRead16(0x4000024) : LcdRead16(0x4000034);

	int size = affine_table[BGinfo.tile_size];
	int pitch = size / 8;
	u8 tile_index, color_index;
	u16 color = 0;

	for (int x = start; x < end; x++)
	{
		s32 aff_x = refx >> 8;
		s32 aff_y = refy >> 8;
		refx += dx;
		refy += dy;

		if (BGinfo.affine_wrap)
		{
			//aff_x %= size;
			//aff_y %= size;
		}
		else
		{
			//excess area is transparent
			if ((aff_x < 0) || (aff_y < 0) || (aff_x > size) || (aff_y > size))
				continue;
		}

		int tile_x = aff_x / 8;
		int tile_y = aff_y / 8;
		u8 fine_x = aff_x % 8;
		u8 fine_y = aff_y % 8;

		u32 sb_address = BGinfo.screen_base * 0x800 + (tile_y * pitch + tile_x);
		LcdReadVram(sb_address, tile_index);
		u32 char_base_address = BGinfo.char_base * 0x4000 + tile_index * 0x40;
		u32 char_address = char_base_address + (fine_y * 8 + fine_x);
		LcdReadVram(char_address, color_index);
		LcdReadPal(color_index * 2, color);

		bg_pixel_data pixel{ BGinfo.mode, BGinfo.priority, color };
		HandleBackgroundPriority(pixel, scanline * 240 + x);
	}
}

void Lcd::BG_normalCalc(const int start, const int end, const int y)
{
	u8 index;
	u16 color = 0;
	u16 pitch = (background_size_x[BGinfo.tile_size]) / 8;
	u16 tile_data;

	for (int x = start; x < end; x++)
	{
		u32 current_y = y + BGinfo.bg_vofs;
		u32 current_x = x + BGinfo.bg_hofs;

		current_x %= background_size_x[BGinfo.tile_size];
		current_y %= background_size_y[BGinfo.tile_size];

		int tile_y = current_y / 8;
		int tile_x = current_x / 8;
		u16 fine_y = current_y % 8;
		u16 fine_x = current_x % 8;

		u32 sb_base_address = (BGinfo.screen_base + (tile_y / 32) * (pitch / 32) + (tile_x / 32)) * 0x800;
		u32 sb_address = sb_base_address + ((tile_y % 32) * 32 + (tile_x % 32)) * 2;
		LcdReadVram(sb_address, tile_data);

		u16 tile_pal = tile_data >> 12;
		u16 tile_index = tile_data & 0x3FF;
		bool hflip = tile_data & 0x400;
		bool vflip = tile_data & 0x800;
		if (vflip) fine_y = 7 - fine_y;
		if (hflip) fine_x = 7 - fine_x;

		u32 char_base_address = BGinfo.char_base * 0x4000 + tile_index * 0x20;
		u32 char_address = char_base_address + (((fine_y * 4) + (fine_x / 2)) << BGinfo.color_mode);
		LcdReadVram(char_address, index);

		if (!BGinfo.color_mode)
		{
			int pixel_num = fine_x % 2;
			index = (index & (0x0F << (4 * pixel_num))) >> (4 * pixel_num);
			LcdReadPal((tile_pal * 16 * 2) + index * 2, color);
		}
		else
		{
			LcdReadPal(index * 2, color);
		}

		bg_pixel_data pixel{ BGinfo.mode, BGinfo.priority, color };
		if (index != 0) HandleBackgroundPriority(pixel, y * 240 + x);
	}
}

void Lcd::Sprite_setAttr(int start, int end, int scanline, obj_attr attr_data, bool& drawSprite)
{
	OBJinfo.attr_y0 = attr_data.attr0 & 0xFF;
	OBJinfo.attr_om = (attr_data.attr0 & 0x0300) >> 8;
	OBJinfo.attr_gm = (attr_data.attr0 & 0x0C00) >> 10;
	OBJinfo.attr_mos = (attr_data.attr0 & 0x1000) >> 12;
	OBJinfo.attr_cm = (attr_data.attr0 & 0x2000) >> 13;
	OBJinfo.attr_sh = (attr_data.attr0 & 0xC000) >> 14;

	OBJinfo.attr_x0 = attr_data.attr1 & 0x01FF;
	OBJinfo.attr_ai = (attr_data.attr1 & 0x3E00) >> 9;
	OBJinfo.attr_hf = (attr_data.attr1 & 0x1000) >> 12;
	OBJinfo.attr_vf = (attr_data.attr1 & 0x2000) >> 13;
	OBJinfo.attr_sz = (attr_data.attr1 & 0xC000) >> 14;

	OBJinfo.attr_id = attr_data.attr2 & 0x03FF;
	OBJinfo.attr_pi = (attr_data.attr2 & 0x0C00) >> 10;
	OBJinfo.attr_pb = (attr_data.attr2 & 0xF000) >> 12;

	OBJinfo.size_x = pixels_x[OBJinfo.attr_sz][OBJinfo.attr_sh];
	OBJinfo.size_y = pixels_y[OBJinfo.attr_sz][OBJinfo.attr_sh];

	OBJinfo.double_aff = OBJinfo.attr_om == 0x3;

	//account for sprite scrolling
	if (OBJinfo.attr_x0 >= 240) OBJinfo.attr_x0 -= 512;
	if (OBJinfo.attr_y0 >= 160) OBJinfo.attr_y0 -= 256;

	//determines whether the sprite is visble or not
	drawSprite =
		((scanline < (OBJinfo.attr_y0 + (OBJinfo.size_y << OBJinfo.double_aff))) // is it greater than lower y bound 
			&& (scanline >= OBJinfo.attr_y0)) // is it less than the upper y bound
		&& (OBJinfo.attr_om != 0x2) //disable sprite flag
		&& ((OBJinfo.attr_x0 + (OBJinfo.size_x << OBJinfo.double_aff)) > (start + 1)) // is it greater than the lower x bound
		&& (OBJinfo.attr_x0 < (end - 1)); // is it less than the upper x bound
}

void Lcd::DrawSprites(u16 posX, u16 scanline, u16 endX, int type)
{
	memcpy(obj_oam_info, busPtr->OAM.mem.data(), sizeof(obj_oam_info));

	bool mapping = LcdRead16(REG_DISPCNT) & 0x40;

	for (int i = 0; i < 128; i++)
	{
		//get sprite attribute data and determine whether it can be drawn or not
		bool canSpritesBeDrawn = false;
		Sprite_setAttr(posX, endX, scanline, obj_oam_info[i], canSpritesBeDrawn);

		if (!canSpritesBeDrawn) continue; //sprite can't be drawn, move on

		switch (OBJinfo.attr_om)
		{
		case 0x00:

			//normal sprite
			Sprite_normalCalc(posX, endX, scanline, i, mapping);
			break;

		case 0x01:

			//affine sprite
			Sprite_affineCalc(posX, endX, scanline, i, mapping);
			break;

		case 0x02:

			//do nothing, sprite is hidden
			break;

		case 0x03:
			//double affine area sprite
			Sprite_affineCalc(posX, endX, scanline, i, mapping);
			break;
		}
	}
}

void Lcd::Sprite_normalCalc(int posX, int endX, int scanline, int oam_index, bool mapping)
{
	int sprite_x, sprite_y, fine_x, fine_y;

	for (int x = 0; x < (OBJinfo.size_x << OBJinfo.double_aff); x++)
	{
		if (((OBJinfo.attr_x0 + x) < (endX - 1)) && ((OBJinfo.attr_x0 + x) > (posX + 1)))
		{
			//get offsets into 8 by 8 pixel bitmap
			fine_x = x % 8;
			fine_y = (scanline - OBJinfo.attr_y0) % 8;

			//get sprite tile coordinates
			sprite_x = x / 8;
			sprite_y = (scanline - OBJinfo.attr_y0) / 8;

			if (OBJinfo.attr_vf)
			{
				sprite_y = (OBJinfo.size_y / 8) - 1 - sprite_y;
				fine_y = 7 - fine_y;
			}

			if (OBJinfo.attr_hf)
			{
				sprite_x = (OBJinfo.size_x / 8) - 1 - sprite_x;
				fine_x = 7 - fine_x;
			}

			//calculate sprite address
			u8 byte = Sprite_indexCalc(sprite_x, sprite_y, fine_x, fine_y, mapping);
			u16 color = 0;

			if (!OBJinfo.attr_cm)
			{
				int pixel_num = fine_x % 2;
				byte = (byte & (0x0F << (4 * pixel_num))) >> (4 * pixel_num);
				color = LcdRead16(0x05000200 + OBJinfo.attr_pb * 16 * 2 + byte * 2);
			}
			else
			{
				color = LcdRead16(0x5000200 + byte * 2);
			}

			obj_pixel_data pixel{ oam_index, OBJinfo.attr_pi, color };
			if (byte != 0)HandleSpritePriority(pixel, scanline * 240 + OBJinfo.attr_x0 + x);
		}
	}
}

void Lcd::Sprite_affineCalc(int posX, int endX, int scanline, int oam_index, bool mapping)
{
	s16 temp_x, temp_y;
	s32 temp_var1, temp_var2;
	float aff_x, aff_y, copy_affx, copy_affy;

	int sprite_x, sprite_y;
	u8 fine_x, fine_y;

	s16 pa, pb, pc, pd;
	LcdReadOam(0x06 + (OBJinfo.attr_ai) * sizeof(obj_attr) * 4, pa);
	LcdReadOam(0x0E + (OBJinfo.attr_ai) * sizeof(obj_attr) * 4, pb);
	LcdReadOam(0x16 + (OBJinfo.attr_ai) * sizeof(obj_attr) * 4, pc);
	LcdReadOam(0x1E + (OBJinfo.attr_ai) * sizeof(obj_attr) * 4, pd);

	temp_x = -((OBJinfo.size_x << OBJinfo.double_aff) / 2);
	temp_y = scanline - OBJinfo.attr_y0 - ((OBJinfo.size_y << OBJinfo.double_aff) / 2);

	temp_var1 = (temp_x * pa + temp_y * pb);
	temp_var2 = (temp_x * pc + temp_y * pd);

	aff_x = (float)temp_var1 / 256;
	aff_y = (float)temp_var2 / 256;

	aff_x += (OBJinfo.size_x / 2);
	aff_y += (OBJinfo.size_y / 2);

	copy_affx = aff_x, copy_affy = aff_y;

	for (int x = 0; x < (OBJinfo.size_x << OBJinfo.double_aff); x++)
	{
		if (((OBJinfo.attr_x0 + x) < (endX - 1)) && ((OBJinfo.attr_x0 + x) > (posX + 1)))
		{
			// x is already relative to the sprite start(attr_x0, attr_y0)
			aff_x = copy_affx;
			aff_y = copy_affy;

			copy_affx += ((float)pa / 256);
			copy_affy += ((float)pc / 256);

			if ((aff_x < 0) || (aff_x >= OBJinfo.size_x) || (aff_y < 0) || (aff_y >= OBJinfo.size_y))
				continue;

			//get offsets into 8 by 8 pixel bitmap
			fine_x = (int)(aff_x) % 8;
			fine_y = (int)(aff_y) % 8;

			//get sprite tile coordinates
			sprite_x = (int)(aff_x) / 8;
			sprite_y = (int)(aff_y) / 8;

			//calculate sprite address
			u8 byte = Sprite_indexCalc(sprite_x, sprite_y, fine_x, fine_y, mapping);
			u16 color = 0;

			if (!OBJinfo.attr_cm)
			{
				int pixel_num = fine_x % 2;
				byte = (byte & (0x0F << (4 * pixel_num))) >> (4 * pixel_num);
				LcdReadPal(0x200 + (OBJinfo.attr_pb * 32) + byte * 2, color);
			}
			else
			{
				LcdReadPal(0x200 + byte * 2, color);
			}

			obj_pixel_data pixel{ oam_index, OBJinfo.attr_pi, color };
			if (byte != 0)HandleSpritePriority(pixel, scanline * 240 + OBJinfo.attr_x0 + x);
		}
	}
}

u8 Lcd::Sprite_indexCalc(int sprite_x, int sprite_y, int fine_x, int fine_y, bool mapping)
{
	u32 base_address = 0;
	if (mapping)
	{
		base_address += sprite_y * (OBJinfo.size_x / 8);
	}
	else
	{
		base_address += sprite_y * (OBJinfo.attr_cm ? 16 : 32);
	}
	base_address += sprite_x;
	base_address *= (OBJinfo.attr_cm ? 0x40 : 0x20);
	base_address += 0x6010000 + OBJinfo.attr_id * 0x20;
	u32 address = base_address + (OBJinfo.attr_cm ? (fine_y * 8) + fine_x : (fine_y * 4) + (fine_x / 2));
	return LcdRead8(address);
}

void Lcd::drawWindow(u32 VideoData, u16 scanline)
{
	bool win0_en = VideoData & 0x2000;
	bool win1_en = VideoData & 0x4000;

	//if any window is enabled, that means there's a winout
	//first draw winout, then win1, win0 to deal with priority

	if (win0_en || win1_en)
	{
		u8 winout_data = LcdRead16(REG_WINOUT) & 0xFF;
		if (winout_data & 1) DrawBackground(0, scanline, 240, 1, 0);
		if (winout_data & 2) DrawBackground(0, scanline, 240, 1, 1);
		if (winout_data & 4)
		{
			if (((VideoData & 7) == 1) || ((VideoData & 7) == 2)) BGaffine = true;
			DrawBackground(0, scanline, 240, 1, 2);
		}
		if (winout_data & 8)
		{
			if ((VideoData & 7) == 2) BGaffine = true;
			DrawBackground(0, scanline, 240, 1, 3);
		}
		if (winout_data & 16) DrawSprites(0, scanline, 240, 1);
	}

	if (win1_en)
	{
		u8 win1_data = (LcdRead16(REG_WININ) >> 8) & 0xFF;
		u8 win1_left = (LcdRead16(REG_WIN1H) >> 8) & 0xFF;
		u8 win1_right = LcdRead16(REG_WIN1H) & 0xFF;
		u8 win1_top = (LcdRead16(REG_WIN1V) >> 8) & 0xFF;
		u8 win1_bot = LcdRead16(REG_WIN1V) & 0xFF;
		if ((scanline >= win1_top) && (scanline < win1_bot))
		{
			if (win1_data & 1) DrawBackground(win1_left, scanline, win1_right, 1, 0);
			if (win1_data & 2) DrawBackground(win1_left, scanline, win1_right, 1, 1);
			if (win1_data & 4) {
				if (((VideoData & 7) == 1) || ((VideoData & 7) == 2)) BGaffine = true;
				DrawBackground(win1_left, scanline, win1_right, 1, 2);
			}
			if (win1_data & 8) {
				if ((VideoData & 7) == 2) BGaffine = true;
				DrawBackground(win1_left, scanline, win1_right, 1, 3);
			}
			if (win1_data & 16) DrawSprites(win1_left, scanline, win1_right, 1);
		}
	}

	if (win0_en)
	{
		u8 win0_data = LcdRead16(REG_WININ) & 0xFF;
		u8 win0_left = (LcdRead16(REG_WIN0H) >> 8) & 0xFF;
		u8 win0_right = LcdRead16(REG_WIN0H) & 0xFF;
		u8 win0_top = (LcdRead16(REG_WIN0V) >> 8) & 0xFF;
		u8 win0_bot = LcdRead16(REG_WIN0V) & 0xFF;
		if ((scanline >= win0_top) && (scanline < win0_bot))
		{
			if (win0_data & 1) DrawBackground(win0_left, scanline, win0_right, 1, 0);
			if (win0_data & 2) DrawBackground(win0_left, scanline, win0_right, 1, 1);
			if (win0_data & 4)
			{
				if (((VideoData & 7) == 1) || ((VideoData & 7) == 2)) BGaffine = true;
				DrawBackground(win0_left, scanline, win0_right, 1, 2);
			}
			if (win0_data & 8) {
				if ((VideoData & 7) == 2) BGaffine = true;
				DrawBackground(win0_left, scanline, win0_right, 1, 3);
			}
			if (win0_data & 16) DrawSprites(win0_left, scanline, win0_right, 1);
		}
	}
}

void Lcd::HandleSpritePriority(obj_pixel_data new_pixel, const u16 location)
{
	bool write = false;

	obj_pixel_data old_pixel = sprite_buffer[location];

	if (new_pixel.priority < old_pixel.priority)
	{
		write = true;
	}
	else if ((old_pixel.priority == new_pixel.priority) && (new_pixel.oam_index < old_pixel.oam_index))
	{
		write = true;
	}

	if (write) sprite_buffer[location] = new_pixel;
}

void Lcd::HandleBackgroundPriority(bg_pixel_data new_pixel, const u16 location)
{
	bool write = false;
	bg_pixel_data old_pixel = background_buffer[location];

	if (new_pixel.priority < old_pixel.priority)
	{
		write = true;
	}
	else if ((old_pixel.priority == new_pixel.priority) && (new_pixel.mode < old_pixel.mode))
	{
		write = true;
	}

	if (write) {
		background_buffer[location] = new_pixel;
	}
}

void Lcd::HandleBackgroundSpritePriority(const u16 scanline)
{
	u16 backdrop_color = LcdRead16(0x5000000);

	obj_pixel_data def;

	bg_pixel_data m;

	for (int i = 0; i < 240; i++)
	{
		u16 color = 0xffff;

		bg_pixel_data bg = background_buffer[scanline * 240 + i];
		obj_pixel_data obj = sprite_buffer[scanline * 240 + i];

		if (obj.priority <= bg.priority)
			color = obj.color;
		else
			color = bg.color;

		if ((bg.priority == 100) && (obj.priority == 50))
			color = backdrop_color;

		pixels[scanline * 240 + i] = color;
		sprite_buffer[scanline * 240 + i] = def;
		background_buffer[scanline * 240 + i] = m;
	}
}

void Lcd::ClearBuffer() { pixels.fill(0); }

void Lcd::DrawFrame()
{
	SDL_UpdateTexture(texture, 0, pixels.data(), width * sizeof(Uint16));
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texture, 0, 0);
	SDL_RenderPresent(renderer);
}

Lcd::~Lcd()
{
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyTexture(texture);
	window = NULL;
	renderer = NULL;
	texture = NULL;
	SDL_Quit();
}
