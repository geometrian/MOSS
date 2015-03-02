#include "includes.h"

#include "kernel/graphics/vga/terminal.h"
#include "kernel/kernel.h"

#include "mossc/cstdio"


extern "C" void die(void) {
	//Clear hardware interrupts (to prevent the next instruction being interrupted) and then "hlt".
	__asm__ __volatile__("cli");
	__asm__ __volatile__("hlt");
	//If that fails, busy loop.
	LOOP: goto LOOP;
}

namespace MOSS {


void _message(char const* filename,int line, char const* fmt_cstr,va_list args) {
	Graphics::VGA::Terminal::Color temp = kernel->terminal->color_text;
	kernel->terminal->set_color_text(Graphics::VGA::Terminal::COLOR_RED);

	kernel->write("(%s:%d): ", filename,line);

	char buffer[1024];
	MOSSC::vsprintf(buffer, fmt_cstr,args); //TODO: vsnprintf
	kernel->write("%s", buffer);

	kernel->write("\n");

	kernel->terminal->set_color_text(temp);
}


}
