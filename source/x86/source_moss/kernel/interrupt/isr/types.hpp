#pragma once

#include "../../../includes.hpp"


namespace MOSS { namespace Interrupts {


//See Intel Manual, section 6.13.
//	This error code works for all exceptions that push error codes except page faults (see note in section)
class ErrorCode final { public:
	bool              external_event :  1; //1=The exception occurred during delivery of an event external to the program, such as an interrupt or an earlier exception.
	bool              descr_location :  1; //0=index refers to a descriptor in the GDT or LDT, 1=index refers to a gate descriptor in the IDT.
	bool                descr_in_LDT :  1; //Used iff descr_location=0.  0=index refers to descriptor in GDT, 1=index refers to segment/gate descriptor in LDT.
	uint16_t segement_selector_index : 13; //Current manual seems to imply this is the right size (the "upper half" is reserved, and it's 32 bits big altogether).  Confirmed by http://ftp.utcluj.ro/pub/users/nedevschi/PMP/protected86/protgemode/translate_c242.html.
	uint16_t                reserved : 16;
} __attribute__((packed));
static_assert(sizeof(ErrorCode)==sizeof(uint32_t),"Implementation error!");
//Page faults are special (see around pg. 285)
class ErrorCodePF final { public:
	bool            P :  1; //Caused by 0=non-present page, 1=page-level protection violation
	bool           WR :  1; //Access causing fault was 0=read, 1=write
	bool           US :  1; //Access causing fault happened while running in 0=supervisor mode, 1=user mode
	bool         RSVD :  1; //0=The fault was not caused by reserved bit violation, 1=The fault was caused by reserved bits set to 1 in a page directory
	bool           ID :  1; //Whether the fault was caused by an instruction fetch
	uint32_t reserved : 27;
} __attribute__((packed));
static_assert(sizeof(ErrorCodePF)==sizeof(uint32_t),"Implementation error!");


//This encapsulates the state of the processor after it gets interrupted, including the original
//	values of the registers.  Refer to the explanation in "isr-delegator.hpp" and code in
//	"isr-delegator.asm".
class InterruptState final {
	public:
		//Step 6: We push the segment registers
		uint32_t gs, fs, es, ds;
		//Step 5: We push all general-purpose registers ("pusha").  Yes; in this order:
		//	EAX, ECX, EDX, EBX, ESP (from before this pushing EAX), EBP, ESI, EDI
		//	See http://x86.renejeschke.de/html/file_module_x86_id_270.html
		uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
		//Step 4: We push a debug value and jump to the common subroutine.
		uint32_t debug_marker;
		//Step 3: We push the interrupt's index
		uint32_t int_index;
		//Step 2: We push one of the error code types.
		union {
			ErrorCode standard;
			ErrorCodePF page_fault;
			uint32_t packed;
		} error_code;
		//Step 1: CPU pushes automatically when interrupt fires.
		uint32_t eip, cs, eflags, useresp, ss;

	public:
		void write(void) const;
};


}}
