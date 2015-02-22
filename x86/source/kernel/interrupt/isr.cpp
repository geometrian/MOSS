#include "isr.h"

#include "../text_mode_terminal.h"


namespace MOSS { namespace Interrupts {


extern "C" void isr_handler(void) {
	/*Kernel::terminal->write("received interrupt:\n");
	Kernel::terminal->write(get_interrupt_description(regs.int_no)); Kernel::terminal->write("\n");
	Kernel::terminal->write((int)(regs.err_code)); Kernel::terminal->write("\n");
	Kernel::terminal->write("DONE\n");*/
	Kernel::terminal->write("received interrupt!\n");
}


}}