#pragma once

#include "../stdinc.h"

namespace IDT {

//"__attribute__((packed))" tells GCC not to change any of the alignment in the structure.

//A struct describing an interrupt gate.
struct idt_entry {
	uint16 base_lo; //The lower 16 bits of the address to jump to when this interrupt fires.
	uint16     sel; //Kernel segment selector.
	uint8  always0; //This must always be zero.
	uint8    flags; //More flags. See documentation.
	uint16 base_hi; //The upper 16 bits of the address to jump to.
} __attribute__((packed));

//A struct describing a pointer to an array of interrupt handlers.  This is in a format suitable for giving to 'lidt'.
struct idt_ptr {
	uint16 limit;
	uint32  base; //The address of the first element in our idt_entry_t array.
} __attribute__((packed));

extern "C" void idt_lidt(uint32); //ASM routine

#define ISR(N) extern "C" void isr##N(void);
	ISR( 0) ISR( 1) ISR( 2) ISR( 3) ISR( 4) ISR( 5) ISR( 6) ISR( 7) ISR( 8) ISR( 9)
	ISR(10) ISR(11) ISR(12) ISR(13) ISR(14) ISR(15) ISR(16) ISR(17) ISR(18) ISR(19)
	ISR(20) ISR(21) ISR(22) ISR(23) ISR(24) ISR(25) ISR(26) ISR(27) ISR(28) ISR(29)
	ISR(30) ISR(31)
#undef ISR

static void idt_set_gate(idt_entry* entry, uint32 base, uint16 sel, uint8 flags) {
	entry->base_lo = base & 0xFFFF;
	entry->base_hi = (base >> 16) & 0xFFFF;

	entry->    sel = sel;
	entry->always0 =   0;

	//We must uncomment the OR below when we get to using user-mode.  It sets the interrupt gate's privilege level to 3.
	entry->flags = flags /*| 0x60*/;
}

static idt_entry idt_entries[256];
static idt_ptr ptr;

static void idt_load(void) {
	for (int i=0;i<(int)(sizeof(idt_entry)*256);++i) {
		*( ((char*)(idt_entries)) + i ) = 0;
	}

	ptr.limit = sizeof(idt_entry)*256 - 1;
	ptr.base  = (uint32)(&idt_entries);

	#define IDT_SET_GATE(N) idt_set_gate(idt_entries+N, (uint32)(isr##N), 0x08, 0x8E);
		IDT_SET_GATE( 0) IDT_SET_GATE( 1) IDT_SET_GATE( 2) IDT_SET_GATE( 3) IDT_SET_GATE( 4) IDT_SET_GATE( 5)
		IDT_SET_GATE( 6) IDT_SET_GATE( 7) IDT_SET_GATE( 8) IDT_SET_GATE( 9) IDT_SET_GATE(10) IDT_SET_GATE(11)
		IDT_SET_GATE(12) IDT_SET_GATE(13) IDT_SET_GATE(14) IDT_SET_GATE(15) IDT_SET_GATE(16) IDT_SET_GATE(17)
		IDT_SET_GATE(18) IDT_SET_GATE(19) IDT_SET_GATE(20) IDT_SET_GATE(21) IDT_SET_GATE(22) IDT_SET_GATE(23)
		IDT_SET_GATE(24) IDT_SET_GATE(25) IDT_SET_GATE(26) IDT_SET_GATE(27) IDT_SET_GATE(28) IDT_SET_GATE(29)
		IDT_SET_GATE(30) IDT_SET_GATE(31)
	#undef IDT_SET_GATE

	idt_lidt( (uint32)(&ptr) );
}

}