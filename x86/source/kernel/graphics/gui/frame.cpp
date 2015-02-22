#include "frame.h"

#include "../../input/mouse.h"

#include "../color.h"

#include "../vesa/framebuffer.h"

#include "button.h"


namespace MOSS { namespace Graphics { namespace GUI {


Frame::Frame(ComponentBase* parent, int x,int y, int w,int h) : ComponentBase(parent,Rect(x,y,w,h),Rect(x,y,w,h)) {
	set_position(x,y);
	set_size(w,h);

	button_close = new Buttons::ButtonClose(this);

	visible = true; //TODO: false
}
Frame::~Frame(void) {
	delete button_close;
}

void Frame::set_position(int x, int y) {
	rect_component.x = x;
	rect_component.y = y;
}
void Frame::set_size(int width, int height) {
	rect_component.w =  width;
	rect_component.h = height;
	rect_client.w =  width;
	rect_client.h = height;
}

void Frame::set_title(const MOSST::String& title) {
	this->title = title;
}

void Frame::set_visible(bool visible) {
	this->visible = visible;
}

void Frame::draw(VESA::Framebuffer* framebuffer) const {
	framebuffer->draw_rect(
		rect_component.x,rect_component.y,rect_component.w,rect_component.h,
		Graphics::Color(64,64,64,255)
	);

	framebuffer->draw_text(
		rect_component.x+10,rect_component.y+rect_component.h-10,
		title.c_str(),
		Graphics::Color(255,255,255)
	);

	button_close->draw(framebuffer);
}


}}}