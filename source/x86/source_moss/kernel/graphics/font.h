#pragma once

#include "../../includes.h"


namespace MOSS { namespace Graphics { namespace Font {


/*//256 characters, each 8x8 pixel-bits, packed into a single 8-byte long long (uint64_t) each
extern uint64_t font[256 * 8*8 / 8];*/


class Character8x8 final { public:
	union {
		uint8_t rows[8];
		uint64_t data;
	};

	void print(int x, int y) const;
};
static_assert(sizeof(Character8x8)==8*8/(8*sizeof(uint8_t)),"Character8x8 is wrong size!");

class Character8x16 final { public:
	uint8_t rows[16];

	void print(int x, int y) const;
};
static_assert(sizeof(Character8x16)==8*16/(8*sizeof(uint8_t)),"Character8x16 is wrong size!");


//256 characters, each 8x8 pixel bitmaps
extern Character8x8 const* font8x8;

//256 characters, each 8x16 pixel bitmaps
extern Character8x16 const* font8x16;


}}}
