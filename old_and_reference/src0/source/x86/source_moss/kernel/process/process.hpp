#pragma once

#include "../../includes.hpp"

#include "../../mossc/cstdlib"
#include "../../mosst/linked_list.hpp"

#include "../interrupt/isr/types.hpp"

#include "../kernel.hpp"


namespace MOSS { namespace Process {


//http://wiki.osdev.org/Kernel_Multitasking


inline void main0(void) {
	while (1) {
		//MOSS_DEBUG_BOCHSBREAK;
		kernel->write("A");
		//for (int i=0;i<1000;++i) for (int j=0;j<1000;++j);
	}
}
inline void main1(void) {
	while (1) {
		kernel->write("B");
		//for (int i=0;i<1000;++i) for (int j=0;j<1000;++j);
	}
}


inline uint8_t* alloc_page(void) {
	return reinterpret_cast<uint8_t*>(MOSSC::malloc(4*1024));
}


class ProcessRecord final {
	public:
		class StateRegisters final {
			public:
				uint32_t eax, ebx, ecx, edx;
				uint32_t esi, edi;
				uint32_t esp, ebp;
				uint32_t eip;
				//uint32_t eflags;
				//uint32_t cr3;

			public:
				inline StateRegisters(/*uint32_t flags,*/ void(*main)(void)) :
					eax(0u), ebx(0u), ecx(0u), edx(0u),
					esi(0u), edi(0u),
					eip(reinterpret_cast<uint32_t>(main))//,
					//eflags(flags),
					//cr3( (uint32_t)pagedir )
				{
					uint32_t stack = (uint32_t)alloc_page() + 4*1024;
					esp = ebp = stack;
					static_assert(sizeof( void(*)(void) )==sizeof(uint32_t),"Implementation error!");
				}
				inline ~StateRegisters(void) = default;

				inline void load(Interrupts::InterruptState const* old_state) const;
				void save(Interrupts::InterruptState const* state) {
					eax=state->eax; ebx=state->ebx; ecx=state->ecx; edx=state->edx;
					esi=state->esi; edi=state->edi;
					esp=state->esp; ebp=state->ebp;
					eip=state->eip;
					//eflags=state->eflags;
				}

				void print(void) const {
					kernel->write("this: 0x%p\n",this);
					kernel->write("eip: 0x%p\n",eip);//, eflags: %u, useresp: %u\n",eip,eflags,useresp);
					kernel->write("eax: 0x%p    edi: 0x%p\n",eax,edi);
					kernel->write("ebx: 0x%p    esi: 0x%p\n",ebx,esi);
					kernel->write("ecx: 0x%p    ebp: 0x%p\n",ecx,ebp);
					kernel->write("edx: 0x%p    esp: 0x%p\n",edx,esp);
				}
		} regs;

		void(*const main)(void);

	public:
		explicit inline ProcessRecord( void(*main)(void) ) : regs(main), main(main) {}
		//inline ProcessRecord(uint32_t flags, void(*main)(void)) : regs(flags,main), main(main) {}
		inline ~ProcessRecord(void) = default;
};

extern "C" void _load_process(
	Interrupts::InterruptState const* old_state,
	ProcessRecord::StateRegisters const* new_regs
);
inline void ProcessRecord::StateRegisters::load(Interrupts::InterruptState const* old_state) const {
	kernel->write("old, this:   0x%p, 0x%p\n",old_state,this);
	_load_process(old_state,this);
}


//Assembly function that takes the currently running state and stuffs it into "from", and then
//	loads and begins executing the state stored in "to".
extern "C" void switch_process(
	ProcessRecord::StateRegisters*restrict from,
	ProcessRecord::StateRegisters*restrict   to
);


class Scheduler final {
	public:
		//MOSST::LinkedList<ProcessRecord> processes;
		ProcessRecord process0;
		ProcessRecord process1;

		ProcessRecord* current;

	public:
		inline Scheduler(void) :
			process0( main0 ),
			process1( main1 ),
			current(nullptr)
		{
			//processes.insert_back(ProcessRecord(0u,process0));
			//processes.insert_back(ProcessRecord(0u,process1));
		}
		inline ~Scheduler(void) = default;

		void step(Interrupts::InterruptState const* state) {
			//state->write(); while (1);
			//kernel->write("swap!\n");
			if (current!=nullptr) {
				/*//kernel->write("Step 1\n");
				current->regs.save(state);
				//kernel->write("Step 2\n");
				if (current==&process0) {
					kernel->write("(0->1)");
					current = &process1;
				} else {
					kernel->write("(1->0)");
					current = &process0;
				}*/
			} else {
				//kernel->write("swap (first)!\n");
				//kernel->write("Step 3\n");
				current = &process0;

				current->regs.print();
				//MOSS_DEBUG_BOCHSBREAK;
				current->regs.load(state);
			}
			//kernel->write("Step 4\n");
			//current->regs.load(state);
			//kernel->write("Step 5\n");

			//kernel->write("swap");
			//switch_process();
		}
};


}}
