#pragma once

#include "../stdafx.h"



/*
The GDT is a necessary structure for x86 modes higher than 16-bit real mode.  It describes memory
segmentation (even if, as is the common case, it's a flat memory model).

There is far, *far* too much to explain here.  Refer to the GDT / LDT / Segmentation document and
Exceptions / Interrupts document in "doc/".
*/



/*
GDT / IDT Descriptor

Describes the GDT or IDT to the CPU, intuitively as a pointer and a size.  The base address
specifies the (linear) address of the table.  The limit is the maximum index of any byte in the
table (i.e. 1 less than the size).

See Intel Arch Manual Vol. 3A §2.4.1 "Global Descriptor Table Register (GDTR)"
See Intel Arch Manual Vol. 3A §6.10 "Interrupt Descriptor Table (IDT)"
https://wiki.osdev.org/Global_Descriptor_Table#GDTR
*/
union _DescrTable64_Info
{
	#pragma pack(push,1)
	struct { uint16_t limit; uint64_t base_addr; };
	#pragma pack(pop)
	uint8_t data[ 2 + 8 ];
};

static_assert( sizeof(union _DescrTable64_Info) == 2+8 );



/*
Setup a flat memory model global descriptor table (GDT)

Note: interrupts must be off before calling, and of course you need to be in ring 0.

`x86_real16_setup_gdt1632()`
	Setup a 16-/32-bit GDT from 16-bit real mode.

`x86_prot32_setup_gdt1632()`
	Setup a 16-/32-bit GDT from 32-bit protected mode.  This weird application is necessary because
	some bootloaders give you long mode but don't specify what the GDT they used actually *is*.

`x86_longmode64_setup_gdt64()`
	Setup a 64-bit GDT from long mode code.  Reason is as above.
*/
//void x86_core_real16_setup_gdt1632(void);
//void x86_core_prot32_setup_gdt1632(void);
void x86_core_longmode64_setup_gdt64(void);



//void x86_core_setup_ivt16(void);
//void x86_core_setup_idt32(void);
void x86_core_setup_idt64(void);



MOSS_INLINE void x86_test_cause_div_zero(void) { asm("xor bx, bx\ndiv bx"); }
