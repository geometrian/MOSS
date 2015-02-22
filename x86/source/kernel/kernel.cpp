#include "kernel.h"

#include "graphics/gui/manager.h"
#include "graphics/color.h"
#include "graphics/vesa/controller.h"

#include "../mossc/cstdio"
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
	Graphics::GUI::Manager* gui = NULL;
#else
	Terminal::TextModeTerminal* terminal;
	Memory::MemoryManager* memory;
	Input::Devices::ControllerPS2* controller;
	Graphics::VESA::Controller* graphics;
	Graphics::GUI::Manager* gui;
#endif

void handle_key_down(const Input::Keys::Event& event) {
	if (Input::Keys::is_printable(event.key)) {
		//terminal->write("Keyboard got scan code %d!\n",scan_code);
		terminal->write(Input::Keys::get_printable(event.key));
	}
}
void handle_key_up(const Input::Keys::Event&/* event*/) {

}

void handle_mouse_move(const Input::Mouse::EventMove& event) {
	/*//terminal->write("Mouse position: %d, %d\n",event.x,event.y);

	graphics->set_pixel(event.x,event.y, Graphics::Color(255u,0u,255u,255u));

	char buffer[64];
	MOSSC::sprintf(buffer,"Mouse position: (%+5d,%+5d)",event.x,event.y);

	graphics->draw_text(50,50, buffer, Graphics::Color(0u,255u,255u),Graphics::Color(32u,32u,32u));
	//graphics->draw_text(50,50, "Hello World!", Graphics::Color(0u,255u,255u));*/
	gui->handle_mouse(event);
}

void kernel_main(void) {
	/*char buffer[256];
	graphics->current_mode->get_printable(buffer);
	graphics->draw_text(50,graphics->current_mode->info.YResolution-50, buffer, Graphics::Color(255u,255u,255u));

	//terminal->write("Hanging in kernel!\n");

	//graphics->fill(Graphics::Color(255u,0u,0u,255u));

	graphics->draw_text(50,70, "TEST WRITING!", Graphics::Color(0u,255u,255u),Graphics::Color(32u,32u,32u));

	int x = 20;
	int y = 600;
	for (int r=0;r<255;r+=64) {
		for (int g=0;g<255;g+=64) {
			for (int b=0;b<255;b+=64) {
				MOSSC::sprintf(buffer,"=(%3d,%3d,%3d)",r,g,b);
				graphics->draw_text(x+110,y, buffer, Graphics::Color(255u,255u,255u),Graphics::Color(32u,32u,32u));
				graphics->draw_rect(x,y,100,10, Graphics::Color(r,g,b));
				y -= 15;
				if (y<100) {
					y = 600;
					x += 250;
				}
			}
		}
	}*/

	/*for (int r=0;r<=255;++r) {
		for (int g=0;g<=255;++g) {
			graphics->set_pixel(5+r,5+g, Graphics::Color(255,0,0));
		}
	}*/

	gui->add_frame("Hello world!", 50,50, 300,200);

	while (true) {
		Kernel::graphics->frame_start();

		gui->draw(Kernel::graphics->current_framebuffer);

		Kernel::graphics->frame_end();
		Kernel::graphics->frame_flip();
	}
}


}}