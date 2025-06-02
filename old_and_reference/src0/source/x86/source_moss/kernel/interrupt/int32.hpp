#pragma once

#include "../../includes.hpp"


namespace MOSS { namespace Interrupts {


class regs16_t final { public:
	uint16_t di;
	uint16_t si;

	uint16_t bp;
	uint16_t sp;

	union {
		uint16_t bx;
		struct { unsigned char bl; unsigned char bh; }; //note little-endian
	};
	union {
		uint16_t dx;
		struct { unsigned char dl; unsigned char dh; }; //note little-endian
	};
	union {
		uint16_t cx;
		struct { unsigned char cl; unsigned char ch; }; //note little-endian
	};
	union {
		uint16_t ax;
		struct { unsigned char al; unsigned char ah; }; //note little-endian
	};

	uint16_t gs;
	uint16_t fs;
	uint16_t es;
	uint16_t ds;

	uint16_t eflags;
} __attribute__((packed));


//http://www.rohitab.com/discuss/topic/35103-switch-between-real-mode-and-protected-mode/
extern "C" void int32(unsigned char intnum, regs16_t* regs);

/*//int32 test
void int32_test(void) {
	regs16_t regs;

	// switch to 320x200x256 graphics mode
	regs.ax = 0x0013;
	int32(0x10, &regs);

	// full screen with blue color (1)
	memset((char*)(0xA0000), 1, (320*200));

	// draw horizontal line from 100,80 to 100,240 in multiple colors
	for (int y=0; y<200; ++y) {
		memset((char*)(0xA0000) + (y*320+80), y, 160);
	}

	// wait for key
	regs.ax = 0x0000;
	int32(0x16, &regs);

	// switch to 80x25x16 text mode
	regs.ax = 0x0003;
	int32(0x10, &regs);
}*/


}}
