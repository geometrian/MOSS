#include "gdt_idt.h"

#include "../text_mode_terminal.h"


namespace MOSS { namespace Interrupts {


EntryGDT::Access::AccessByte EntryGDT::Access::AccessByte::get_null(void) {
	EntryGDT::Access::AccessByte result;
	result.  accessed = 0;
	result.        rw = 0;
	result.  dir_conf = 0;
	result.discr_type = 0;
	result.    unused = 0;
	result. privilege = 0;
	result.   present = 0;
	return result;
}
EntryGDT::Access::AccessByte EntryGDT::Access::AccessByte::get_selector_datacode(uint8_t ring) {
	EntryGDT::Access::AccessByte result;
	result.  accessed =    0;
	result.        rw =    1;
	result.  dir_conf =    0;
	result.discr_type =    1;
	result.    unused =    1;
	result. privilege = ring;
	result.   present =    1;
	return result;
}

void EntryGDT::set_limit(uint32_t limit) {
	limit_low  =  limit & 0x0FFFF     ;
	limit_high = (limit & 0xF0000)>>16;
}
uint32_t EntryGDT::get_limit(void) const {
	return (limit_high<<16) | limit_low;
}

void EntryGDT::set_base(uint32_t base) {
	base_low  =  base & 0x00FFFFFF     ;
	base_high = (base & 0xFF000000)>>24;
}
uint32_t EntryGDT::get_base(void) const {
	return (base_high<<24) | base_low;
}

void EntryGDT::construct(EntryGDT* entry, uint32_t base, uint32_t limit, Access::AccessByte access) {
	entry->set_base(base);
	entry->set_limit(limit);

	entry->access.flags = access;

	entry->     flags_unused = 0;
	entry->       flags_size = 1; //32-bit mode segment
	entry->flags_granularity = 0; //The limit is a 32-bit number and represents the number of bytes, NOT the number of 4KiB blocks
}


void EntryIDT::set_offset(uint32_t offset) {
	offset_low  =  offset & 0x0000FFFF     ;
	offset_high = (offset & 0xFFFF0000)>>16;
}
uint32_t EntryIDT::get_offset(void) const {
	return (offset_high<<16) | offset_low;
}

void EntryIDT::construct(EntryIDT* entry, uint32_t offset, int privilege) {
	entry->set_offset(offset);
	entry->selector = 0x0008;

	entry->unused = 0;

	entry->type_attr.   gate_type =         8;
	entry->type_attr.storage_elem =         1;
	entry->type_attr.   privilege = privilege;
	entry->type_attr.     present =         0;
}


#define MOSS_NUM_GDT 5
static EntryGDT gdt_entries[MOSS_NUM_GDT];
void load_gdt(void) {
	uint32_t base  = (uint32_t)(&gdt_entries);          //The linear address of the first gdt_entry_t struct.
	uint16_t limit = sizeof(EntryGDT)*MOSS_NUM_GDT - 1; //The upper 16 bits of all selector limits.  Size of table minus 1.  sizeof(EntryGDT) should be 8.

	#ifdef MOSS_DEBUG
	moss_assert(sizeof(EntryGDT)==8,"EntryGDT is the wrong size!");
	#endif

	EntryGDT::construct(gdt_entries,   0x00000000u,         0u, EntryGDT::Access::AccessByte::              get_null()); //Null segment
	EntryGDT::construct(gdt_entries+1, 0x00000000u,0xFFFFFFFFu, EntryGDT::Access::AccessByte::get_selector_datacode(0)); //Kernel code segment
	EntryGDT::construct(gdt_entries+2, 0x00000000u,0xFFFFFFFFu, EntryGDT::Access::AccessByte::get_selector_datacode(0)); //Kernel data segment
	EntryGDT::construct(gdt_entries+3, 0x00000000u,0xFFFFFFFFu, EntryGDT::Access::AccessByte::get_selector_datacode(3)); //User code segment
	EntryGDT::construct(gdt_entries+4, 0x00000000u,0xFFFFFFFFu, EntryGDT::Access::AccessByte::get_selector_datacode(3)); //User data segment

	gdt_lgdt(base,limit);
}
#undef MOSS_NUM_GDT


/*static EntryIDT idt_entries[256];
void load_idt(void) {
	uint32_t base  = (uint32_t)(&idt_entries);
	uint16_t limit = sizeof(EntryIDT)*256 - 1;

	#define IDT_SET_GATE(N) EntryIDT::construct(idt_entries+N, (uint32_t)(isr), 0);
	MOSS_INTERRUPT(IDT_SET_GATE)
	#undef IDT_SET_GATE

	idt_lidt(base,limit);
}
#undef MOSS_NUM_IDT*/


}}