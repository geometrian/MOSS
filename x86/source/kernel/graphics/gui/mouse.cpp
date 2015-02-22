#include "mouse.h"

#include "../vesa/framebuffer.h"

#include "../color.h"


namespace MOSS { namespace Graphics { namespace GUI {


Mouse::Mouse(void) : x(0),y(0) {
}
Mouse::~Mouse(void) {
}

void Mouse::draw(VESA::FrameBuffer* framebuffer) const {
	framebuffer->draw_rect(x,y,20,20, Color(255,255,255));
}


}}}