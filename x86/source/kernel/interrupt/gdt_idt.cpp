#include "gdt_idt.h"

#include "isr.h"
#include "../text_mode_terminal.h"


namespace MOSS { namespace Interrupts {


EntryGDT::Access::AccessByte EntryGDT::Access::AccessByte::get_null(void) {
	EntryGDT::Access::AccessByte result;
	result.  accessed = 0;
	result.        rw = 0;
	result.  dir_conf = 0;
	result.executable = 0;
	result.    unused = 0;
	result. privilege = 0;
	result.   present = 0;
	return result;
}
EntryGDT::Access::AccessByte EntryGDT::Access::AccessByte::get_selector_code(uint8_t ring) {
	EntryGDT::Access::AccessByte result;
	result.  accessed =    0;
	result.        rw =    1;
	result.  dir_conf =    0;
	result.executable =    1;
	result.    unused =    1;
	result. privilege = ring;
	result.   present =    1;
	return result;
}
EntryGDT::Access::AccessByte EntryGDT::Access::AccessByte::get_selector_data(uint8_t ring) {
	EntryGDT::Access::AccessByte result;
	result.  accessed =    0;
	result.        rw =    1;
	result.  dir_conf =    0;
	result.executable =    0;
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

	entry->    flags_unused1 = 0;
	entry->    flags_unused2 = 0;
	entry->       flags_size = 1; //32-bit mode segment
	entry->flags_granularity = 1; //The limit is a 32-bit number and represents the number of 4KiB blocks
}


void EntryIDT::set_offset(uint32_t offset) {
	offset_low  =  offset & 0x0000FFFF     ;
	offset_high = (offset & 0xFFFF0000)>>16;
}
uint32_t EntryIDT::get_offset(void) const {
	return (offset_high<<16) | offset_low;
}

void EntryIDT::construct(EntryIDT* entry, uint32_t offset, Type::TypeByte::GateType type, int privilege) {
	entry->set_offset(offset);
	entry->selector = 0x0008; //Location of kernel code segment in GDT

	entry->unused1 = 0;
	entry->unused2 = 0;

	entry->type.flags.gate_type    =      type;
	//OSDev seems to *imply* this
	/*switch (type) {
		case Type::TypeByte::Interrupt16:
		case Type::TypeByte::Interrupt32:
			entry->type.flags.storage_elem =         0;
			break;
		case Type::TypeByte::     Trap16:
		case Type::TypeByte::     Trap32:
		case Type::TypeByte::     Task32:
			entry->type.flags.storage_elem =         1;
			break;
		//Hopefully the compiler will give a warning if other enum types are possible
	}*/
	//. . . but BrokenThorn *states* this
	entry->type.flags.storage_elem =         0;
	entry->type.flags.   privilege = privilege;
	entry->type.flags.     present =         1;
}


#define MOSS_NUM_GDT 5
static EntryGDT gdt_entries[MOSS_NUM_GDT];
void load_gdt(void) {
	uint32_t base  = (uint32_t)(&gdt_entries);          //The linear address of the first gdt_entry_t struct.
	uint16_t limit = sizeof(EntryGDT)*MOSS_NUM_GDT - 1; //The upper 16 bits of all selector limits.  Size of table minus 1.  sizeof(EntryGDT) should be 8.

	#ifdef MOSS_DEBUG
	moss_assert(sizeof(EntryGDT)==8,"EntryGDT is the wrong size!");
	#endif

	EntryGDT::construct(gdt_entries,   0x00000000u,         0u, EntryGDT::Access::AccessByte::          get_null()); //Null segment
	EntryGDT::construct(gdt_entries+1, 0x00000000u,0xFFFFFFFFu, EntryGDT::Access::AccessByte::get_selector_code(0)); //Kernel code segment
	EntryGDT::construct(gdt_entries+2, 0x00000000u,0xFFFFFFFFu, EntryGDT::Access::AccessByte::get_selector_data(0)); //Kernel data segment
	EntryGDT::construct(gdt_entries+3, 0x00000000u,0xFFFFFFFFu, EntryGDT::Access::AccessByte::get_selector_code(3)); //User code segment
	EntryGDT::construct(gdt_entries+4, 0x00000000u,0xFFFFFFFFu, EntryGDT::Access::AccessByte::get_selector_data(3)); //User data segment

	/*//http://www.brokenthorn.com/Resources/OSDev8.html
	Kernel::terminal->write("  Checking match\n");
	//11111111 11111111 00000000 00000000 00000000 10011010 11001111 00000000
	moss_assert( ((unsigned char*)(gdt_entries))[ 8]==0xFF, "Mismatch on GDT byte 1");
	moss_assert( ((unsigned char*)(gdt_entries))[ 9]==0xFF, "Mismatch on GDT byte 2");
	moss_assert( ((unsigned char*)(gdt_entries))[10]==0x00, "Mismatch on GDT byte 3");
	moss_assert( ((unsigned char*)(gdt_entries))[11]==0x00, "Mismatch on GDT byte 4");
	moss_assert( ((unsigned char*)(gdt_entries))[12]==0x00, "Mismatch on GDT byte 5");
	moss_assert( ((unsigned char*)(gdt_entries))[13]==0x9A, "Mismatch on GDT byte 6");
	moss_assert( ((unsigned char*)(gdt_entries))[14]==0xCF, "Mismatch on GDT byte 7");
	moss_assert( ((unsigned char*)(gdt_entries))[15]==0x00, "Mismatch on GDT byte 8");
	Kernel::terminal->write("  Matched code segment!\n");
	//11111111 11111111 00000000 00000000 00000000 10010010 11001111 00000000
	moss_assert( ((unsigned char*)(gdt_entries))[16]==0xFF, "Mismatch on GDT byte 1");
	moss_assert( ((unsigned char*)(gdt_entries))[17]==0xFF, "Mismatch on GDT byte 2");
	moss_assert( ((unsigned char*)(gdt_entries))[18]==0x00, "Mismatch on GDT byte 3");
	moss_assert( ((unsigned char*)(gdt_entries))[19]==0x00, "Mismatch on GDT byte 4");
	moss_assert( ((unsigned char*)(gdt_entries))[20]==0x00, "Mismatch on GDT byte 5");
	moss_assert( ((unsigned char*)(gdt_entries))[21]==0x92, "Mismatch on GDT byte 6");
	moss_assert( ((unsigned char*)(gdt_entries))[22]==0xCF, "Mismatch on GDT byte 7");
	moss_assert( ((unsigned char*)(gdt_entries))[23]==0x00, "Mismatch on GDT byte 8");
	Kernel::terminal->write("  Matched data segment!\n");*/

	gdt_lgdt(base,limit);
}
#undef MOSS_NUM_GDT


static EntryIDT idt_entries[256];
void load_idt(void) {
	uint32_t base  = (uint32_t)(&idt_entries);
	uint16_t limit = sizeof(EntryIDT)*256 - 1;

	//Kernel::terminal->write((int)(sizeof(EntryIDT)));
	#ifdef MOSS_DEBUG
	moss_assert(sizeof(EntryIDT)==8,"EntryIDT is the wrong size!");
	moss_assert(sizeof(Interrupts::ErrorCode)==4,"Interrupt error code (normal type) is the wrong size!");
	moss_assert(sizeof(Interrupts::ErrorCodePF)==4,"Interrupt error code (for page faults) is the wrong size!");
	#endif

#define IDT_SET_GATE(N) EntryIDT::construct(idt_entries+N, (uint32_t)(isr##N##_asm), EntryIDT::Type::TypeByte::Interrupt32, 0);
	MOSS_INTERRUPT(IDT_SET_GATE)
	#undef IDT_SET_GATE

	idt_lidt(base,limit);
}


}}