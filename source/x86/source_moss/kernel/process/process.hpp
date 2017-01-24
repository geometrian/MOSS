#pragma once

#include "../../includes.hpp"

#include "../../mosst/linked_list.hpp"


namespace MOSS {


//http://wiki.osdev.org/Kernel_Multitasking


class ProcessRecord final {
	public:
		class StateRegisters final {
			public:
				uint32_t eax, ebx, ecx, edx, esi, edi, esp, ebp, eip, eflags, cr3;
		} regs;
};

class Scheduler final {
	public:
		MOSST::LinkedList<ProcessRecord> processes;

	public:
		inline Scheduler(void) = default;
		inline ~Scheduler(void) = default;
};


}
