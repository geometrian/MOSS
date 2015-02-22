#pragma once

#include "../ui/console.h"

#include "../stdinc.h"

namespace ISR {

typedef struct registers {
	uint32 ds;                                     //Data segment selector
	uint32 edi, esi, ebp, esp, ebx, edx, ecx, eax; //Pushed by pusha
	uint32 int_no, err_code;                       //Interrupt number and error code (if applicable)
	uint32 eip, cs, eflags, useresp, ss;           //Pushed by the processor automatically
} registers_t;

const char* get_interrupt_description(int num) {
	if (num<0) return "Invalid exception!";
	else if (num<=18) {
		switch (num) {
			case  0: return "Division by zero exception";
			case  1: return "Debug exception";
			case  2: return "Non maskable interrupt";
			case  3: return "Breakpoint exception";
			case  4: return "\"Into detected overflow\"";
			case  5: return "Out of bounds exception";
			case  6: return "Invalid opcode exception";
			case  7: return "No coprocessor exception";
			case  8: return "Double fault (pushes an error code)";
			case  9: return "Coprocessor segment overrun";
			case 10: return "Bad TSS (pushes an error code)";
			case 11: return "Segment not present (pushes an error code)";
			case 12: return "Stack fault (pushes an error code)";
			case 13: return "General protection fault (pushes an error code)";
			case 14: return "Page fault (pushes an error code)";
			case 15: return "Unknown interrupt exception";
			case 16: return "Coprocessor fault";
			case 17: return "Alignment check exception";
			case 18: return "Machine check exception";
		}
	}
	else if (num<=31) return "Reserved";
	else if (num<=255) return "Available";
	return "Invalid exception!";
}

extern "C" void isr_handler(registers_t regs) {
	CONSOLE::Console::draw(5,5,"received interrupt: ");
	CONSOLE::Console::draw(5,6,get_interrupt_description(regs.int_no));
	CONSOLE::Console::draw(5,7,"DONE");
}

}