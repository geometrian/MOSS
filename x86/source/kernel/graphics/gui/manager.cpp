#include "manager.h"

#include "../color.h"
#include "../vesa/framebuffer.h"

#include "frame.h"
#include "mouse.h"


namespace MOSS { namespace Graphics { namespace GUI {


Manager::Manager(void) {
	mouse = new Mouse();
}
Manager::~Manager(void) {
	delete mouse;

	while (frames.get_size()>0) {
		delete frames.pop_back();
	}
}

void Manager::set_mouse_position(int x, int y) {
	mouse->x = x;
	mouse->y = y;
}

void Manager::add_frame(const MOSST::String& title, int x,int y,int w,int h) {
	Frame* frame = new Frame(x,y,w,h);
	frame->set_title(title);

	frames.push_back(frame);
}

void Manager::draw(VESA::FrameBuffer* framebuffer) const {
	framebuffer->draw_fill(Color(0,0,0));

	for (int i=0;i<frames.get_size();++i) {
		frames[i]->draw(framebuffer);
	}

	mouse->draw(framebuffer);
}


}}}