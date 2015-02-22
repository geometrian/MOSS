#include "includes.h"

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
	kernel->write("(%s:%d): ", filename,line);

	char buffer[256];
	MOSSC::vsprintf(buffer, fmt_cstr,args); //TODO: vsnprintf
	kernel->write("%s", buffer);

	kernel->write("\n");
}


}
