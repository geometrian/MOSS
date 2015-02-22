#include "gdt.h"


namespace MOSS { namespace Memory {


//GDT Entry
class EntryGDT final {
	private:
		uint32_t        limit_low : 16; //The lower 16 bits of the limit
		uint32_t         base_low : 24; //The lower 24 bits of base
	public:
		//Access byte used in GDT entries
		//	See http://files.osdev.org/mirrors/geezer/os/pm.htm:
		//		executable bit: 1 is code/data selector, 0 is TSS, LDT, or Gate (but also says earlier the same as the below):
		//	See http://wiki.osdev.org/Global_Descriptor_Table:
		//		executable bit: 1 is code selector, 0 is data selector
		//	See http://wiki.osdev.org/Segmentation
		//		executable bit: 1 is code/data selector, 0 is system
		//	I am going to use 1=code selector/0=data selector version, since doing it the other way causes a triple fault.
		union Access {
			class AccessByte final { public:
				bool     accessed :  1; //Initialized to 0; CPU sets when segment is accessed
				bool           rw :  1; //Writable bit for data selectors / Readable bit for code selectors
				bool     dir_conf :  1; //For data selectors, 0 the segment grows up, 1 the segment grows down.  For code selectors, 0 only executable by processes with exactly privilege, 1 lower is okay too
				bool   executable :  1; //See above
				bool       unused :  1; //Actually usused?  Initialized to 1 (Descriptor Bit; see http://www.brokenthorn.com/Resources/OSDev8.html)
				uint8_t privilege :  2; //Ring
				bool      present :  1; //Must be 1 for all valid selectors

				static AccessByte get_null(void);
				static AccessByte get_selector_code(uint8_t ring);
				static AccessByte get_selector_data(uint8_t ring);
			} flags; //Will be packed since EntryGDT is packed?
			uint8_t          byte :  8;
		} access;
	private:
		uint32_t       limit_high :  4; //The upper 4 bits of the limit
	public:
		//Flags.  Can't be in a struct because it is not byte-aligned and it would screw up the packing.
		int         flags_unused1 :  1; //Can be reserved for OS use
		int         flags_unused2 :  1; //unused (at least on x86).  Initialized to 0.
		bool           flags_size :  1; //0 is 16-bit protected mode, 1 is 32-bit protected mode
		bool    flags_granularity :  1; //If 0 the limit is in 1B blocks (byte granularity), if 1 the limit is in 4KiB blocks (page granularity).
	private:
		uint32_t        base_high :  8; //The upper 8 bits of the base.

	public:
		void set_limit(uint32_t limit);
		uint32_t get_limit(void) const;

		void set_base(uint32_t base);
		uint32_t get_base(void) const;

		static void construct(EntryGDT* entry, uint32_t base, uint32_t limit, Access::AccessByte access);
} __attribute__((packed));

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


#define MOSS_NUM_GDT 5
static EntryGDT gdt_entries[MOSS_NUM_GDT];
void load_gdt(void) {
	uint32_t base  = (uint32_t)(&gdt_entries);          //The linear address of the first gdt_entry_t struct.
	uint16_t limit = sizeof(EntryGDT)*MOSS_NUM_GDT - 1; //The upper 16 bits of all selector limits.  Size of table minus 1.  sizeof(EntryGDT) should be 8.

	assert_term(sizeof(EntryGDT)==8,"EntryGDT is the wrong size!");

	EntryGDT::construct(gdt_entries,   0x00000000u,         0u, EntryGDT::Access::AccessByte::          get_null()); //Null segment
	EntryGDT::construct(gdt_entries+1, 0x00000000u,0xFFFFFFFFu, EntryGDT::Access::AccessByte::get_selector_code(0)); //Kernel code segment
	EntryGDT::construct(gdt_entries+2, 0x00000000u,0xFFFFFFFFu, EntryGDT::Access::AccessByte::get_selector_data(0)); //Kernel data segment
	EntryGDT::construct(gdt_entries+3, 0x00000000u,0xFFFFFFFFu, EntryGDT::Access::AccessByte::get_selector_code(3)); //User code segment
	EntryGDT::construct(gdt_entries+4, 0x00000000u,0xFFFFFFFFu, EntryGDT::Access::AccessByte::get_selector_data(3)); //User data segment

	/*//http://www.brokenthorn.com/Resources/OSDev8.html
	kernel->write("  Checking match\n");
	//11111111 11111111 00000000 00000000 00000000 10011010 11001111 00000000
	moss_assert( ((unsigned char*)(gdt_entries))[ 8]==0xFF, "Mismatch on GDT byte 1");
	moss_assert( ((unsigned char*)(gdt_entries))[ 9]==0xFF, "Mismatch on GDT byte 2");
	moss_assert( ((unsigned char*)(gdt_entries))[10]==0x00, "Mismatch on GDT byte 3");
	moss_assert( ((unsigned char*)(gdt_entries))[11]==0x00, "Mismatch on GDT byte 4");
	moss_assert( ((unsigned char*)(gdt_entries))[12]==0x00, "Mismatch on GDT byte 5");
	moss_assert( ((unsigned char*)(gdt_entries))[13]==0x9A, "Mismatch on GDT byte 6");
	moss_assert( ((unsigned char*)(gdt_entries))[14]==0xCF, "Mismatch on GDT byte 7");
	moss_assert( ((unsigned char*)(gdt_entries))[15]==0x00, "Mismatch on GDT byte 8");
	kernel->write("  Matched code segment!\n");
	//11111111 11111111 00000000 00000000 00000000 10010010 11001111 00000000
	moss_assert( ((unsigned char*)(gdt_entries))[16]==0xFF, "Mismatch on GDT byte 1");
	moss_assert( ((unsigned char*)(gdt_entries))[17]==0xFF, "Mismatch on GDT byte 2");
	moss_assert( ((unsigned char*)(gdt_entries))[18]==0x00, "Mismatch on GDT byte 3");
	moss_assert( ((unsigned char*)(gdt_entries))[19]==0x00, "Mismatch on GDT byte 4");
	moss_assert( ((unsigned char*)(gdt_entries))[20]==0x00, "Mismatch on GDT byte 5");
	moss_assert( ((unsigned char*)(gdt_entries))[21]==0x92, "Mismatch on GDT byte 6");
	moss_assert( ((unsigned char*)(gdt_entries))[22]==0xCF, "Mismatch on GDT byte 7");
	moss_assert( ((unsigned char*)(gdt_entries))[23]==0x00, "Mismatch on GDT byte 8");
	kernel->write("  Matched data segment!\n");*/

	gdt_lgdt(base,limit);
}
#undef MOSS_NUM_GDT


}}