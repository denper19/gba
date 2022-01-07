#pragma once
#include <algorithm>
#include <array>
#include <vector>
#include <cmath>
#include <iostream>
#include <SDL.h>
//#include "C:\Users\Laxmi\OneDrive\Documents\Dev\SDL2-2.0.14\include\SDL.h"
#undef main

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using s16 = std::int16_t;
using s32 = std::int32_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

constexpr int width = 240;

constexpr int height = 160;

#define REG_VCOUNT   0x4000006
#define REG_DISPCNT  0x4000000
#define REG_DISPSTAT 0x4000004
#define REG_IE		 0x4000200
#define REG_IF       0x4000202
#define REG_IME      0x4000208
#define REG_HALT     0x4000301
#define REG_WIN0H    0x4000040
#define REG_WIN1H    0x4000042
#define REG_WIN0V    0x4000044
#define REG_WIN1V	 0x4000046
#define REG_WININ    0x4000048
#define REG_WINOUT	 0x400004a
#define REG_BG2X     0x4000028 
#define REG_BG2Y     0x400002c
#define REG_BG3X     0x4000038 
#define REG_BG3Y     0x400003c

#define REG_BG2X_L	 0x4000028
#define REG_BG2X_H	 0x400002A
#define REG_BG2Y_L	 0x400002C
#define REG_BG2Y_H	 0x400002E

#define REG_BG3X_L	 0x4000038
#define REG_BG3X_H	 0x400003A
#define REG_BG3Y_L	 0x400003C
#define REG_BG3Y_H	 0x400003E

class Gui;

class Bus;

typedef struct obj_attr
{
	u16 attr0;
	u16 attr1;
	u16 attr2;
	s16 pa;
}obj_attr;

typedef struct bg_pixel_data
{
	int mode = 4;
	int priority = 100;
	u16 color = 0xffff;
}bg_pixel_data;

typedef struct obj_pixel_data
{
	int oam_index = 255;
	int priority = 50; //setting it to a high number so that all sprites can write over it
	u16 color = 0xffff;
}obj_pixel_data;

typedef struct
{
	s32 ref_x, ref_y;
	s16 dx, dy;
	s16 dmx, dmy;
} BG_AFF_DATA;

typedef struct
{
	u8 priority;
	u8 char_base;
	u8 mosaic_en;
	u8 color_mode;
	u8 screen_base;
	u8 affine_wrap;
	u8 tile_size;
	u16 bg_hofs;
	u16 bg_vofs;
	u8 mode;
} BG_CNT_DATA;

typedef struct
{
	int attr_y0;
	int attr_om;
	int attr_gm;
	int attr_mos;
	int attr_cm;
	int attr_sh;

	int attr_x0;
	int attr_ai;
	int attr_hf;
	int attr_vf;
	int attr_sz;

	int attr_id;
	int attr_pi;
	int attr_pb;

	int size_x;
	int size_y;

	int double_aff;

} OBJ_ATTR_DATA;


class Lcd
{
private:

	int Cycles_Per_Line = 0;

	u16 background_data;
	u16 bg_hofs;
	u16 bg_vofs;
	u8 background_priority;
	u8 background_char_base;
	u8 background_mosaic_en;
	u8 background_color_mode;
	u8 background_screen_base;
	u8 background_affine_wrap;
	u8 background_tile_size;

	SDL_Renderer* renderer;
	SDL_Texture* texture;
	bg_pixel_data* background_buffer;
	obj_pixel_data* sprite_buffer;
	obj_attr obj_oam_info[128];
	Bus* busPtr;

	friend class GuiInterface;

public:
	std::array <u16, width* height> pixels;
	std::array <u16, width* height> temp_buf;
	std::array <bool, width* height> ObjWinBuffer{ false };
//	std::array <u16, width* height>  ObjWinPixels;

	s32 internal_x;
	s32 internal_y;

	bool BGaffine = false;
	BG_AFF_DATA BG2aff;
	BG_AFF_DATA BG3aff;
	BG_CNT_DATA BGinfo;
	OBJ_ATTR_DATA OBJinfo;
	SDL_Window* window;

	Lcd();

	void ConnectBus(Bus*);

	u8 LcdRead8(u32);
	u16 LcdRead16(u32);
	u32 LcdRead32(u32);

	u32 ReadRegisters(u32);
	void WriteRegisters(u32, u32);

	template<typename T>
	void LcdReadMmio(const u32 addr, T& value);

	template<typename T>
	void LcdReadOam(const u32 addr, T& value);

	template<typename T>
	void LcdReadVram(const u32 addr, T& value);

	template<typename T>
	void LcdReadPal(const u32 addr, T& value);

	void LcdWrite8(u32, u8);
	void LcdWrite16(u32, u16);

	void DrawBitmapMode3(u16);
	void DrawBitmapMode4(u16);
	void DrawBitmapMode5(u16);
	void DrawSprites(u16, u16, u16, int);
	void DrawBackground(int, int, int, int, int);

	void BG_affineCalc(const int start, const int end, const int y);
	void BG_normalCalc(const int start, const int end, const int y);
	u8 Sprite_indexCalc(int sprite_x, int sprite_y, int fine_x, int fine_y, bool mapping);
	void Sprite_affineCalc(int posX, int endX, int scanline, int oam_index, bool mapping);
	void Sprite_normalCalc(int posX, int endX, int scanline, int oam_index, bool mapping);
	void Sprite_setAttr(int start, int end, int scanline, obj_attr attr_data, bool& drawSprite);

	void HandleSpritePriority(obj_pixel_data, const u16);
	void HandleBackgroundPriority(bg_pixel_data, const u16);
	void HandleBackgroundSpritePriority(const u16);

	void UpdateFramePerLine(u16);
	void drawWindow(u32, u16);

	void stepLcd();
	void ClearBuffer();
	void DrawFrame();

	s32 getBG2X();
	s32 getBG2Y();

	s32 getBG3X();
	s32 getBG3Y();

	~Lcd();
};
