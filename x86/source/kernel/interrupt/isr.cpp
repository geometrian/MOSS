#include "isr.h"

#include "../text_mode_terminal.h"


namespace MOSS { namespace Interrupts {


/*static const char* get_interrupt_description(int num) {
	if (num<0) return "Invalid exception (<0)!";
	else if (num<=18) {
		switch (num) {
			case  0: return "Division by zero exception";
			case  1: return "Debug exception";
			case  2: return "Non maskable interrupt"; //Intel reserved?
			case  3: return "Breakpoint exception";
			case  4: return "\"Into detected overflow\"";
			case  5: return "Out of bounds exception";
			case  6: return "Invalid opcode exception";
			case  7: return "No coprocessor exception";
			case  8: return "Double fault"; //pushes an error code
			case  9: return "Coprocessor segment overrun";
			case 10: return "Bad TSS"; //pushes an error code
			case 11: return "Segment not present"; //pushes an error code
			case 12: return "Stack fault"; //pushes an error code
			case 13: return "General protection fault"; //pushes an error code, triple fault
			case 14: return "Page fault"; //pushes an error code
			case 15: return "Unknown interrupt exception"; //Intel reserved?
			case 16: return "Coprocessor fault";
			case 17: return "Alignment check exception";
			case 18: return "Machine check exception";
		}
	}
	else if (num<=31) return "Reserved";
	else if (num<=255) return "Available";
	return "Invalid exception (>255)!";
}*/

extern "C" void isr_handler(void) {
	/*Kernel::terminal->write("received interrupt:\n");
	Kernel::terminal->write(get_interrupt_description(regs.int_no)); Kernel::terminal->write("\n");
	Kernel::terminal->write((int)(regs.err_code)); Kernel::terminal->write("\n");
	Kernel::terminal->write("DONE\n");*/
	Kernel::terminal->write("received interrupt!\n");
}


}}