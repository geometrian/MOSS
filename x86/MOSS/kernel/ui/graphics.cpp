#include "graphics.h"

namespace GRAPHICS {

#define MEM_VGA ((unsigned char*)(0x000A0000))

void plot_pixel(int x, int y, char color) {
	unsigned short offset = x + 320 * y;
	MEM_VGA[offset] = color;
}

#undef MEM_VGA

}