#include "button.h"

#include "../../../mossc/cstdio"

#include "../../input/mouse.h"

#include "../color.h"
#include "../vesa/framebuffer.h"


#include "../../kernel.h"
#include "../vesa/controller.h"


namespace MOSS { namespace Graphics { namespace GUI { namespace Buttons {


ButtonBase::ButtonBase(ComponentBase* parent, const Rect& rect_button) : ComponentBase(parent,rect_button) {}
ButtonBase::~ButtonBase(void) {}

bool ButtonBase::handle_mouse(const Input::Mouse::EventMove& event) /*override*/ {
	//Kernel::graphics->current_framebuffer->draw_text(200,30, "BUTTON HANDLING MOUSE!", Color(255,0,0));

	if (get_component_rect_world().intersects(event.x,event.y)) {
		return true;
	}
	return false;
}


ButtonToggleBase::ButtonToggleBase(ComponentBase* parent, const Rect& rect_button, bool initial_state) : ButtonBase(parent,rect_button), state(initial_state) {}
ButtonToggleBase::~ButtonToggleBase(void) {}


ButtonSingleBase::ButtonSingleBase(ComponentBase* parent, const Rect& rect_button) : ButtonBase(parent,rect_button) {
	selected = false;
}
ButtonSingleBase::~ButtonSingleBase(void) {}

bool ButtonSingleBase::handle_mouse(const Input::Mouse::EventMove& event) /*override*/ {
	selected = ButtonBase::handle_mouse(event);
	return selected;
}


ButtonClose::ButtonClose(ComponentBase* parent) : ButtonSingleBase(parent,Rect(0,0,20,20)) {}
ButtonClose::~ButtonClose(void) {}

void ButtonClose::draw(VESA::Framebuffer* framebuffer) /*override*/ {
	Rect rect = get_component_rect_world();
	framebuffer->draw_rect(rect.x,rect.y,rect.w,rect.h, selected?Color(255,128,0):Color(255,0,0));

	/*char buffer[256];
	MOSSC::sprintf(buffer,"%d %d %d %d",rect.x,rect.y,rect.w,rect.h);
	framebuffer->draw_text(200,10, buffer, Color(255,0,0));

	framebuffer->draw_text(200,20, selected?"true":"false", Color(255,0,0));*/
}


}}}}