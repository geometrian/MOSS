#include "manager.h"

#include "../../input/mouse.h"

#include "../vesa/framebuffer.h"
#include "../color.h"

#include "frame.h"
#include "mouse.h"


namespace MOSS { namespace Graphics { namespace GUI {


Manager::Manager(void) {
	_mouse = new Mouse();
}
Manager::~Manager(void) {
	delete _mouse;
}

void Manager::update(void) {
	for (auto iter=_frames.cbegin(); iter!=_frames.cend(); ++iter) {
		if (!((*iter)->alive)) {
			delete *iter;
			~iter;
		}
	}
}

void Manager::handle_mouse(Input::Mouse::   EventMouseMove const& event) {
	_mouse->x = event.x;
	_mouse->y = event.y;

	for (auto iter=_frames.cbegin(); iter!=_frames.cend(); ++iter) {
		if ((*iter)->handle_mouse(event)) break;
	}
}
void Manager::handle_mouse(Input::Mouse::  EventMouseClick const& event) {
	_mouse->buttons[event.index] =  true;

	for (auto iter=_frames.cbegin(); iter!=_frames.cend(); ++iter) {
		if ((*iter)->handle_mouse(event)) break;
	}
}
void Manager::handle_mouse(Input::Mouse::EventMouseUnclick const& event) {
	_mouse->buttons[event.index] = false;

	for (auto iter=_frames.cbegin(); iter!=_frames.cend(); ++iter) {
		if ((*iter)->handle_mouse(event)) break;
	}
}

void Manager::add_frame(MOSST::String const& title, int x,int y,int w,int h) {
	Frame* frame = new Frame(nullptr, x,y,w,h);
	frame->set_title(title);

	_frames.insert_back(frame);
}

void Manager::draw(VESA::Framebuffer* framebuffer) const {
	framebuffer->draw_fill(Color(0,0,0));

	for (auto iter=_frames.cbegin(); iter!=_frames.cend(); ++iter) {
		(*iter)->draw(framebuffer);
	}

	_mouse->draw(framebuffer);
}


}}}
