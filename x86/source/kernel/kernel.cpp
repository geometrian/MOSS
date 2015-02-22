#include "kernel.h"

#include "input/keys.h"
#include "text_mode_terminal.h"


namespace MOSS { namespace Kernel {


//The objects these point to must exist (see setup.cpp)
#ifdef MOSS_DEBUG
	Terminal::TextModeTerminal* terminal = NULL;
	Memory::MemoryManager* memory = NULL;
	Input::Devices::ControllerPS2* controller = NULL;
#else
	Terminal::TextModeTerminal* terminal;
	Memory::MemoryManager* memory;
	Input::Devices::ControllerPS2* controller;
#endif

void handle_key_down(Input::Keys::Event& event) {
	if (Input::Keys::is_printable(event.key)) {
		/*Kernel::terminal->write("Keyboard got scan code ");
		Kernel::terminal->write((int)(scan_code));
		Kernel::terminal->write("!\n");*/
		Kernel::terminal->write(Input::Keys::get_printable(event.key));
	}
}
void handle_key_up(Input::Keys::Event&/* event*/) {
}

void kernel_main(void) {
	terminal->write("Hanging in kernel!\n");

	while (true);
}


}}