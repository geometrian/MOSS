#include "button.h"

#include "../color.h"
#include "../vesa/framebuffer.h"

#include "../../../mossc/cstdio"


namespace MOSS { namespace Graphics { namespace GUI { namespace Buttons {


ButtonBase::ButtonBase(ComponentBase* parent, const Rect& rect_button) : ComponentBase(parent,rect_button) {}
ButtonBase::~ButtonBase(void) {}


ButtonToggleBase::ButtonToggleBase(ComponentBase* parent, const Rect& rect_button, bool initial_state) : ButtonBase(parent,rect_button), state(initial_state) {}
ButtonToggleBase::~ButtonToggleBase(void) {}


ButtonSingleBase::ButtonSingleBase(ComponentBase* parent, const Rect& rect_button) : ButtonBase(parent,rect_button) {}
ButtonSingleBase::~ButtonSingleBase(void) {}


ButtonClose::ButtonClose(ComponentBase* parent) : ButtonSingleBase(parent,Rect(0,0,20,20)) {}
ButtonClose::~ButtonClose(void) {}

void ButtonClose::draw(VESA::Framebuffer* framebuffer) /*override*/ {
	Rect rect = get_client_rect_world();
	framebuffer->draw_rect(rect.x,rect.y,rect.w,rect.h, Color(255,0,0));

	char buffer[256];
	MOSSC::sprintf(buffer,"%d %d %d %d",rect.x,rect.y,rect.w,rect.h);
	framebuffer->draw_text(200,10, buffer, Color(255,0,0));
}


}}}}