#pragma once

#include "../stdinc.h"

namespace GDT {

//"__attribute__((packed))" tells GCC not to change any of the alignment in the structure.

//This structure contains the value of one GDT entry.
struct gdt_entry {
	uint16 limit_low;    //The lower 16 bits of the limit.
	uint16 base_low;     //The lower 16 bits of the base.
	uint8  base_middle;  //The next 8 bits of the base.
	uint8  access;       //Access flags, determine what ring this segment can be used in.
	uint8  granularity;
	uint8  base_high;    //The last 8 bits of the base.
} __attribute__((packed));

//This struct describes a GDT pointer. It points to the start of our array of GDT entries, and is in the format required by the lgdt instruction.
struct gdt_ptr {
	uint16 limit;  //The upper 16 bits of all selector limits.
	uint32 base;   //The address of the first gdt_entry_t struct.
} __attribute__((packed));

extern "C" void gdt_lgdt(uint32); //ASM routine

static void gdt_set_gate(gdt_entry* entry, uint32 base, uint32 limit, uint8 access, uint8 gran) {
	entry->   base_low = (base & 0xFFFF);
	entry->base_middle = (base >> 16) & 0xFF;
	entry->  base_high = (base >> 24) & 0xFF;

	entry->  limit_low = (limit & 0xFFFF);
	entry->granularity = (limit >> 16) & 0x0F;
	entry->granularity |= gran & 0xF0;

	entry->     access = access;
}

#define NUM_GDT 5
static gdt_entry gdt_entries[NUM_GDT];
static gdt_ptr ptr;

void gdt_load(void) {
	ptr.limit = (sizeof(gdt_entry)*NUM_GDT) - 1;
	ptr.base  = (uint32)(&gdt_entries);

	gdt_set_gate(gdt_entries,   0, 0, 0, 0);                // Null segment
	gdt_set_gate(gdt_entries+1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // Code segment
	gdt_set_gate(gdt_entries+2, 0, 0xFFFFFFFF, 0x92, 0xCF); // Data segment
	gdt_set_gate(gdt_entries+3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // User mode code segment
	gdt_set_gate(gdt_entries+4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // User mode data segment

	gdt_lgdt( (uint32)(&ptr) );
}
#undef NUM_GDT

}