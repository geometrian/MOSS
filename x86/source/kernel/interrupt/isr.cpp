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


void isr_common(State* state) {
	//See sign-extending about unsigned bytes here: http://forum.osdev.org/viewtopic.php?f=1&t=23998&sid=98cd3b1e6b1256f0dbdb0885e84ba05f&start=15.
	//Shouldn't be an issue since everything pushed is 32 bits.

	Kernel::terminal->write("Received interrupt:\n");

	Kernel::terminal->write("  Interrupt index: "); Kernel::terminal->write((int)(state-> int_ind)); Kernel::terminal->write("\n");

	Kernel::terminal->write("  Error code:      "); Kernel::terminal->write((int)(state->err_code)); Kernel::terminal->write("\n");
	if (state->err_code == 14) {
		Kernel::terminal->write("    Is special error code for page faults (interrupt 14):\n");
		Kernel::terminal->write("    Diagnostic output not implemented!\n"); //TODO: this
	} else {
		Kernel::terminal->write("    Is normal error code for exceptions:\n");
		//state->err_code = 0x200212u;
		ErrorCode code = *((ErrorCode*)(&state->err_code));

		if (code.external_event==0) {
			Kernel::terminal->write("      0 : ?\n");
		} else {
			Kernel::terminal->write("      1 : <External Event>\n");
		}

		if (code.descr_location==0) {
			if (code.descr_in_LDT==0) {
				Kernel::terminal->write("      0 : index into GDT\n");
			} else {
				Kernel::terminal->write("      1 : index into LDT\n");
			}
		} else {
			Kernel::terminal->write("      1 : index is into IDT\n");
		}

		Kernel::terminal->write("      "); Kernel::terminal->write((int)(code.segement_selector_index)); Kernel::terminal->write("\n");
	}
	Kernel::terminal->write("Done.  Hanging . . .\n");
	//Kernel::terminal->write("received interrupt!\n");
	while (true);

	//0b 00010000 01000000 00000100 (0x104004)
	//0b 00010000 01000000 00010100 (0x104014) (1064980)
	//0b 00010000 01000001 10100100 (0x1041A4) (1065380)
	//0b 00010000 01000011 00110100 (0x104334) (1065780)
	//0b 00100000 00000010 00010010 (0x200212) (2097682)
}


}}