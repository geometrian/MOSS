#include "isr.h"

#include "../text_mode_terminal.h"


namespace MOSS { namespace Interrupts {


/*void isr17(ErrorCode error) {
	//Since it is unclear whether this ISR pushed an error code, I'm considering it fatal until I know
	//(since guessing wrong would be a serious bug; the error code must be popped in the underlying
	//assembly code that called this).

	Kernel::terminal->write("Interrupt 17 happened, and I don't know what to do!  See \"" __FILE__ "\"!\n");
	while (true);
}


void isr20through31(void) {
	//I don't know what I'm supposed to do here.  Fatal.
}


void isr32through255(void) {
	//OS use
}*/


void isr_common(const State* state) {
	//Note that the error interrupt index and error code are unsigned bytes, but they're on the stack, and so were sign extended if they
	//happened to be over 127.  See also http://forum.osdev.org/viewtopic.php?f=1&t=23998&sid=98cd3b1e6b1256f0dbdb0885e84ba05f&start=15.
	Kernel::terminal->write("Received interrupt:\n");
	Kernel::terminal->write("  Interrupt index: "); Kernel::terminal->write((int)(state-> int_ind)); Kernel::terminal->write("\n");
	Kernel::terminal->write("  Error code:      "); Kernel::terminal->write((int)(state->err_code)); Kernel::terminal->write("\n");
	Kernel::terminal->write("Done.  Hanging . . .\n");
	//Kernel::terminal->write("received interrupt!\n");
	while (true);

	//0b100000100000000000100 (0x104004)
	//0b100000100000000010100 (0x104014) (1064980)
}


}}