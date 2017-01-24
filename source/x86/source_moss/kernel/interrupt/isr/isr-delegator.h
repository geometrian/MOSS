#pragma once

#include "../../../includes.h"


namespace MOSS { namespace Interrupts {


//When an ISR assembly callback is invoked, it jumps quickly to a common assembly subroutine defined in isr.asm, which then calls this C++ common handler below.
//This function then delegates the event to the appropriate ISR C++ callback defined in isr.cpp.  See isr.asm for more details on this flow.

//This defines what the stack looks like after an ISR was running
class State final { public:
	uint32_t gs, fs, es, ds; //pushed the segs last
	uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; //pushed by "pusha"
	uint32_t debug_marker; //TODO: remove in isr-delegator.asm
	uint32_t int_ind;
	uint32_t err_code; //One of the error code types.  Cast it because unfortunately it can't be the error code type we'll actually use
	uint32_t eip, cs, eflags, useresp, ss; //pushed by the processor automatically
};

//Assembly-callable C++ ISR servicing all IRQs.  This routine delegates to a particular ISR defined in "isrs.h".
extern "C" void isr_delegator_cpp(State const* state);


}}
