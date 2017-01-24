#pragma once

#include "../../../includes.hpp"

#include "types.hpp"


namespace MOSS { namespace Interrupts {


/*
Interrupts come in two types:
	1: "Software interrupt"s / "exception"s.  Includes "fault"s, "trap"s, and "abort"s.  Exceptions
	   are generated internally by the CPU.  They are generally the first 32 interrupt vectors.
	   Assembly code can also generate software interrupts on any vector with the "int"
	   instruction.  This latter method is oft-used for system calls.
	2: "Hardware interrupt"s / "interrupt request"s ("IRQ"s).  Generated externally by a chipset.

When the processor receives an interrupt in MOSS:
	1: "eflags", "cs", "eip", and (possibly) an error code are pushed onto the stack (Intel Manual
	   ~pg. 248).  Iff the interrupt is an IRQ, the CPU also disables interrupts.
	2: A small "ISR" assembly routine is invoked (see "isr-delegator.asm")
	3: If no error code was pushed by the CPU, the small routines push a dummy error code.  Then,
	   they push the interrupt index (and optionally a debug value) and then jump to a common
	   assembly subroutine "isr_delegator_asm" (see "isr-delegator.asm").
	4: This subroutine pushes the general-purpose registers and the rest of the segment registers
	   onto the stack, and then calls the below function "isr_delegator_cpp(...)".  All the pushing
	   translates into passing a nice structure for it to take as an argument.
	5: The below function handles the interrupt, and then returns.
*/


//Assembly-callable C++ ISR servicing all IRQs.  This routine delegates to a particular ISR defined in "isrs.hpp".
extern "C" void isr_delegator_cpp(InterruptState const* state);


}}
