#include "kernel.h"

#include "graphics/vesa.h"

#include "input/devices/controller_ps2.h"
#include "input/devices/mouse.h"
#include "input/keys.h"
#include "input/mouse.h"

#include "text_mode_terminal.h"


namespace MOSS { namespace Kernel {


//The objects these point to must exist (see setup.cpp)
#ifdef MOSS_DEBUG
	Terminal::TextModeTerminal* terminal = NULL;
	Memory::MemoryManager* memory = NULL;
	Input::Devices::ControllerPS2* controller = NULL;
	Graphics::VESA::Controller* graphics = NULL;
#else
	Terminal::TextModeTerminal* terminal;
	Memory::MemoryManager* memory;
	Input::Devices::ControllerPS2* controller;
	Graphics::VESA::Controller* graphics = NULL;
#endif

void handle_key_down(const Input::Keys::Event& event) {
	if (Input::Keys::is_printable(event.key)) {
		/*terminal->write("Keyboard got scan code ");
		terminal->write((int)(scan_code));
		terminal->write("!\n");*/
		terminal->write(Input::Keys::get_printable(event.key));
	}
}
void handle_key_up(const Input::Keys::Event&/* event*/) {
}

void handle_mouse_move(const Input::Mouse::EventMove& event) {
	//terminal->write("Mouse position: "); Kernel::terminal->write(event.x); Kernel::terminal->write(", "); Kernel::terminal->write(event.y); Kernel::terminal->write("\n");
	graphics->set_pixel(event.x,event.y, 255,0,255,255);
	graphics->draw_text(50,50, 
}

void kernel_main(void) {
	terminal->write("Hanging in kernel!\n");

	while (true);
}


}}