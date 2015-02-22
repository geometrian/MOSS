#pragma once
#include "../../includes.h"


namespace MOSS { namespace Interrupts {


//See Intel Manual, section 6.13.
//This error code works for all exceptions that push error codes except page faults (see note in section)
class ErrorCode { public:
	bool              external_event :  1; //1=The exception occurred during delivery of an event external to the program, such as an interrupt or an earlier exception.
	bool              descr_location :  1; //0=index refers to a descriptor in the GDT or LDT, 1=index refers to a gate descriptor in the IDT.
	bool                descr_in_LDT :  1; //Used iff descr_location=0.  0=index refers to descriptor in GDT, 1=index refers to segment/gate descriptor in LDT.
	uint16_t segement_selector_index : 13; //Current manual seems to imply this is the right size (the "upper half" is reserved, and it's 32 bits big altogether).  Confirmed by http://ftp.utcluj.ro/pub/users/nedevschi/PMP/protected86/protgemode/translate_c242.html.
	uint16_t                reserved : 16;
} __attribute__((packed));
//Page faults are special (see around pg. 285)
class ErrorCodePF { public:
	bool            P :  1; //Caused by 0=non-present page, 1=page-level protection violation
	bool           WR :  1; //Access causing fault was 0=read, 1=write
	bool           US :  1; //Access causing fault happened while running in 0=supervisor mode, 1=user mode
	bool         RSVD :  1; //0=The fault was not caused by reserved bit violation, 1=The fault was caused by reserved bits set to 1 in a page directory
	bool           ID :  1; //Whether the fault was caused by an instruction fetch
	uint32_t reserved : 27;
} __attribute__((packed));


typedef bool(*ISR)(void);
class IQR_ISR { public:
	ISR def; //the default ISR
	ISR isr; //Any overriding ISR
	bool operator()(void) const;
};

extern IQR_ISR irq_isrs[16];

void init_irqs(void);


//When an ISR assembly callback is invoked, it jumps quickly to a common assembly subroutine defined in isr.asm, which then calls this C++ common handler below.
//This function then delegates the event to the appropriate ISR C++ callback defined in isr.cpp.  See isr.asm for more details on this flow.

//This defines what the stack looks like after an ISR was running
class State { public:
	uint32_t gs, fs, es, ds; //pushed the segs last
	uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; //pushed by "pusha"
	uint32_t debug_marker; //TODO: remove in isr.asm
	uint32_t int_ind;
	uint32_t err_code; //One of the error code types.  Cast it because unfortunately it can't be the error code type we'll actually use
	uint32_t eip, cs, eflags, useresp, ss; //pushed by the processor automatically
};

extern "C" void isr_common(State* state); //defined in isr.cpp; not assembly TODO: const


}}