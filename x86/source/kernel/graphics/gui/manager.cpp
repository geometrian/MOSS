#include "manager.h"

#include "frame.h"


namespace MOSS { namespace Graphics { namespace GUI {


Manager::Manager(void) {
	mouse_position[0] = 0;
	mouse_position[1] = 0;
}
Manager::~Manager(void) {
	while (frames.get_size()>0) {
		delete frames.pop_back();
	}
}

void Manager::set_mouse_position(int x, int y) {
	mouse_position[0] = x;
	mouse_position[1] = y;
}

void Manager::add_frame(const MOSST::String& title, int x,int y,int w,int h) {
	Frame* frame = new Frame(x,y,w,h);
	frame->set_title(title);

	frames.push_back(frame);
}

void Manager::draw(void) const {
	Kernel::graphics->frame_start();

	Kernel::graphics->current_frame->draw_fill(Color(0,0,0));

	for (int i=0;i<frames.get_size();++i) {
		frames[i]->draw();
	}

	Kernel::graphics->current_frame->draw_rect(mouse_position[0],mouse_position[1],20,20, Color(255,255,255));

	Kernel::graphics->frame_end();
	Kernel::graphics->frame_flip();
}


}}}