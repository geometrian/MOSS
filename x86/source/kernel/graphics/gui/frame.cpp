#include "frame.h"

#include "../color.h"

#include "../vesa/framebuffer.h"


namespace MOSS { namespace Graphics { namespace GUI {


Frame::Frame(int x,int y, int w,int h) {
	set_position(x,y);
	set_size(w,h);
	visible = true; //TODO: false
}
Frame::~Frame(void) {
}

void Frame::set_position(int x, int y) {
	params_window.x = x;
	params_window.y = y;
}
void Frame::set_size(int width, int height) {
	params_window.w =  width;
	params_window.h = height;
}

void Frame::set_title(const MOSST::String& title) {
	this->title = title;
}

void Frame::set_visible(bool visible) {
	this->visible = visible;
}

void Frame::draw(VESA::FrameBuffer* framebuffer) const {
	framebuffer->draw_rect(params_window.x,params_window.y,params_window.w,params_window.h, Graphics::Color(64,64,64,255));
	framebuffer->draw_text(params_window.x+10,params_window.y+params_window.h-10, title.c_str(), Graphics::Color(255,255,255));
}


}}}