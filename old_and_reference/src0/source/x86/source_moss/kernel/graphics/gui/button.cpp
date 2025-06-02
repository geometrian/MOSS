#include "button.hpp"

#include "../../../mossc/cstdio"

#include "../../input/mouse.hpp"
#include "../../kernel.hpp"

#include "../vesa/controller.hpp"
#include "../vesa/framebuffer.hpp"
#include "../color.hpp"


namespace MOSS { namespace Graphics { namespace GUI { namespace Buttons {


ButtonBase::ButtonBase(ComponentBase* parent, const Rect& rect_button) : ComponentBase(parent,rect_button) {
	hovering = false;
}

bool ButtonBase::handle_mouse(const Input::Mouse::EventMouseMove& event) /*override*/ {
	//Kernel::graphics->current_framebuffer->draw_text(200,30, "BUTTON HANDLING MOUSE!", Color(255,0,0));

	if (get_component_rect_world().intersects(event.x,event.y)) {
		hovering = true;

		return true;
	}

	hovering = false;

	return false;
}


ButtonToggleBase::ButtonToggleBase(ComponentBase* parent, const Rect& rect_button, bool initial_state) : ButtonBase(parent,rect_button), state(initial_state) {}


ButtonSingleBase::ButtonSingleBase(ComponentBase* parent, const Rect& rect_button) : ButtonBase(parent,rect_button) {
	selected = false;
}


ButtonClose::ButtonClose(ComponentBase* parent) : ButtonSingleBase(parent,Rect(0,0,20,20)) {}

bool ButtonClose::handle_mouse(const Input::Mouse::EventMouseClick& event) /*override*/ {
	if (!ComponentBase::handle_mouse(event)) {
		if (hovering) parent->alive = false;
		return true;
	}
	return false;
}

void ButtonClose::draw(VESA::Framebuffer* framebuffer) /*override*/ {
	Rect rect = get_component_rect_world();
	framebuffer->draw_rect(rect.x,rect.y,rect.w,rect.h, hovering?Color(255,128,0):Color(255,0,0));

	/*char buffer[256];
	MOSSC::sprintf(buffer,"%d %d %d %d",rect.x,rect.y,rect.w,rect.h);
	framebuffer->draw_text(200,10, buffer, Color(255,0,0));

	framebuffer->draw_text(200,20, selected?"true":"false", Color(255,0,0));*/
}


}}}}
