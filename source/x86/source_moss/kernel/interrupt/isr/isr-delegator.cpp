#include "isr-delegator.hpp"

#include "../pic.hpp"

#include "isrs.hpp"


namespace MOSS { namespace Interrupts {


void isr_delegator_cpp(State const* state) {
	//MOSS_DEBUG_BOCHSBREAK;

	//See sign-extending about unsigned bytes here: http://forum.osdev.org/viewtopic.php?f=1&t=23998&sid=98cd3b1e6b1256f0dbdb0885e84ba05f&start=15.
	//	Shouldn't be an issue since everything pushed is 32 bits.

	uint32_t which = state->int_ind;

	/*kernel->write("Received interrupt:\n");
	kernel->write("  Interrupt index: %d\n",state->int_ind);
	kernel->write("  Error code:      %d\n",state->err_code);
	if (state->err_code == 14) {
		kernel->write("    Is special error code for page faults (interrupt 14):\n");
		kernel->write("    Diagnostic output not implemented!\n"); //TODO: this
	} else {
		kernel->write("    Is normal error code for exceptions:\n");

		union {
			uint32_t data;
			ErrorCode code;
		} convert;
		ErrorCode code = convert.code;

		if (code.external_event==0) {
			kernel->write("      0 : ?\n");
		} else {
			kernel->write("      1 : <External Event>\n");
		}

		if (code.descr_location==0) {
			if (code.descr_in_LDT==0) {
				kernel->write("      0 : index into GDT\n");
			} else {
				kernel->write("      1 : index into LDT\n");
			}
		} else {
			kernel->write("      1 : index is into IDT\n");
		}

		kernel->write("      %d\n",code.segement_selector_index);
	}
	kernel->write("Delegating\n");*/

	//kernel->write("Got interrupt %d!\n",which);

	assert_term(which<=255,"Got interrupt that was outside the ISR table!");
	assert_term(which<= 47,"Got interrupt that was unallocated!");
	if (which>=32) { //remapped IRQ from a PIC
		bool handled = true;
		switch (which) {
			case 32: handled&=isr32(); break;
			case 33: handled&=isr33(); break;
			case 34: isr34(); break;
			case 35: isr35(); break;
			case 36: isr36(); break;
			case 37: isr37(); break;
			case 38: isr38(); break;
			case 39: isr39(); break;
			case 40: isr40(); break;
			case 41: isr41(); break;
			case 42: isr42(); break;
			case 43: isr43(); break;
			case 44: handled&=isr44(); break;
			case 45: isr45(); break;
			case 46: handled&=isr46(); break;
			case 47: handled&=isr47(); break;
		}
		if (handled) {
			//Tell the PIC that we handled the interrupt and that it can send another/reset hardware interrupt at 8259 chip.
			//	The only reason why we wouldn't is if the interrupt was erroneous (spurious, for example).
			Interrupts::PIC::send_EOI(which);
		}
	} else {
		assert_term(which<=19,"Got interrupt from Intel reserved area!");
		switch (which) {
			#define ERRORCODE_ISR_CASE(I,TYPE) case I: {\
				union {\
					uint32_t data;\
					TYPE code;\
				} convert;\
				convert.data = state->err_code;\
				isr##I(convert.code);\
				break;\
			}
			case  0:  isr0(); break;
			case  1:  isr1(); break;
			case  2:  isr2(); break;
			case  3:  isr3(); break;
			case  4:  isr4(); break;
			case  5:  isr5(); break;
			case  6:  isr6(); break;
			case  7:  isr7(); break;
			ERRORCODE_ISR_CASE( 8,  ErrorCode)
			case  9:  isr9(); break;
			ERRORCODE_ISR_CASE(10,  ErrorCode)
			ERRORCODE_ISR_CASE(11,  ErrorCode)
			ERRORCODE_ISR_CASE(12,  ErrorCode)
			ERRORCODE_ISR_CASE(13,  ErrorCode)
			ERRORCODE_ISR_CASE(14,ErrorCodePF)
			case 15: isr15(); break;
			case 16: isr16(); break;
			ERRORCODE_ISR_CASE(17,  ErrorCode)
			case 18: isr18(); break;
			case 19: isr19(); break;
			#undef ERRORCODE_ISR_CASE
		}
	}
}


}}
