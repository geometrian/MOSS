#include "kernel.h"

#include "../mossc/cstdio"

#include "ata/controller.h"

#include "graphics/gui/manager.h"
#include "graphics/vesa/controller.h"
#include "graphics/vesa/framebuffer.h"
#include "graphics/color.h"

#include "input/devices/controller_ps2.h"
//#include "input/devices/mouse_ps2.h"
#include "input/keys.h"
#include "input/mouse.h"

#include "text_mode_terminal.h"


namespace MOSS {


Kernel* kernel;


Kernel::Kernel(void) {
	#ifdef MOSS_DEBUG
		terminal = nullptr;

		graphics = nullptr;
		gui = nullptr;

		controller_ps2 = nullptr;

		controller_ata = nullptr;

		memory = nullptr;
	#endif
}
Kernel::~Kernel(void) {}

void Kernel::handle_key_down(Input::Keys::Event const&/* event*/) {
	/*//terminal->write('A');
	if (Input::Keys::is_printable(event.key)) {
		//write("Keyboard got scan code %d!\n",scan_code);
		write(Input::Keys::get_printable(event.key));
	}*/
}
void Kernel::handle_key_up(Input::Keys::Event const&/* event*/) {
}

void Kernel::handle_mouse_move(Input::Mouse::EventMouseMove const& event) {
	//write("Mouse position: %d, %d\n",event.x,event.y);

	/*graphics->current_framebuffer->set_pixel(event.x,event.y, Graphics::Color(255u,0u,255u,255u));

	char buffer[64];
	MOSSC::sprintf(buffer,"Mouse position: (%+5d,%+5d)",event.x,event.y);

	graphics->current_framebuffer->draw_text(50,50, buffer, Graphics::Color(0u,255u,255u),Graphics::Color(32u,32u,32u));*/
	//graphics->draw_text(50,50, "Hello World!", Graphics::Color(0u,255u,255u));
	gui->handle_mouse(event);
}
void Kernel::handle_mouse_click(Input::Mouse::EventMouseClick const& event) {
	gui->handle_mouse(event);
}
void Kernel::handle_mouse_unclick(Input::Mouse::EventMouseUnclick const& event) {
	gui->handle_mouse(event);
}

void Kernel::write(char const* format, ...) {
	va_list args;
	va_start(args,format);

	terminal->write(format, args);

	va_end(args);
}

void Kernel::init(void) {
	#if 1
		/*for (int i=0;i<graphics->numof_modes;++i) {
			graphics->modes[i]->print(terminal);
			delay(1000);
		}*/
		//Graphics::VESA::Mode* mode = graphics->get_mode_closest( 800,600,32);
		Graphics::VESA::Mode* mode = graphics->get_mode_closest(1024,768,32);
		//Graphics::VESA::Mode* mode = graphics->get_mode_closest(1152,864,32);

		//write("Setting mode "); mode->print(terminal); write(" . . .\n"); delay(5000);
		graphics->set_mode(mode);
		//graphics->set_pixel(5,5, 255,0,0,255);
	#endif
}
void Kernel::main(void) {
	#if 0
		char buffer[256];
		graphics->current_mode->get_printable(buffer);
		graphics->draw_text(50,graphics->current_mode->info.YResolution-50, buffer, Graphics::Color(255u,255u,255u));
	#endif
	#if 0
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
		}
	#endif
	#if 0
		terminal->write("Hanging in kernel!\n"); while (true);
	#endif

	/*for (int r=0;r<=255;++r) {
		for (int g=0;g<=255;++g) {
			graphics->set_pixel(5+r,5+g, Graphics::Color(255,0,0));
		}
	}*/

	gui->add_frame("Hello world!", 50,500, 300,200);

	while (true) {
		gui->update();

		Kernel::graphics->frame_start();

		gui->draw(Kernel::graphics->current_framebuffer);

		Kernel::graphics->frame_end();
		Kernel::graphics->frame_flip();
	}
}


}