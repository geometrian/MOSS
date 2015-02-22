#include "mouse.h"

#include "../vesa/framebuffer.h"

#include "../color.h"


namespace MOSS { namespace Graphics { namespace GUI {


Mouse::Mouse(void) : x(20),y(20) {
}
Mouse::~Mouse(void) {
}

void Mouse::draw(VESA::Framebuffer* framebuffer) const {
	//framebuffer->draw_rect(x,y,20,20, Color(255,255,255));

	const int coordinates[] = {x,y, x,y-11, x+3,y-10, x+5,y-14, x+7,y-13, x+5,y-9, x+7,y-7, x,y};
	//const int coordinates[] = {x,y, x-30,y-30, x+20,y-50, x,y};

	int num = sizeof(coordinates)/sizeof(int)/2;
	for (int i=0;i<num-1;++i) {
		framebuffer->draw_line(
			coordinates[2* i   ],coordinates[2* i   +1],
			coordinates[2*(i+1)],coordinates[2*(i+1)+1],
			Color(255,255,255)
		);
	}
	//framebuffer->draw_line(10,20,100,50, Color(255,255,255));
}


}}}