















namespace MOSS { namespace Memory {


//ASM helper that actually loads the table.
extern "C" void _lgdt(uint32_t base, size_t limit);

/*
GDT Entry
	Refer to diagram:
		https://en.wikipedia.org/wiki/Global_Descriptor_Table
		http://wiki.osdev.org/File:Gdt_entry.png
	The GDT tells the CPU about memory's organization.
	Fields:
		Limit (20 bits): Maximum addressable unit (units are bytes or pages; see flags bit 3)
		Base (32 bits): Address where memory segment begins
		Access byte:
			Accessed bit.  Initialized to 0; set to 1 when CPU accesses.
			Readable/writable bit.  Data should always readable.  Code should always be non-writable.
			Direction bit/conforming bit.
				Data selectors: 0 is segment grows up; 1 is segment grows down.
				Code selectors: 0 only executable by processes with exact privilege; 1 lower is okay too.
			Executable bit:
				See http://files.osdev.org/mirrors/geezer/os/pm.htm:
					executable bit: 1 is code/data selector, 0 is TSS, LDT, or Gate (but also says earlier the same as the below):
				See http://wiki.osdev.org/Global_Descriptor_Table:
					executable bit: 1 is code selector, 0 is data selector
				See http://wiki.osdev.org/Segmentation
					executable bit: 1 is code/data selector, 0 is system
				I am going to use 1=code selector/0=data selector version, since doing it the other way causes a triple fault.
			Privilege: two bits containing ring level; see direction bit for code.
			Present bit: must be 1.
		Flags:
			Bit 0: can be reserved for OS use
			Bit 1: unused (at least on x86).  Initialized to 0.
			Bit 2: 0 is 16-bit protected mode; 1 is 32-bit protected mode
			Bit 3: 0 is 1B blocks (byte granularity); 1 is 4KiB blocks (page granularity).
*/
class EntryGDT final {
	public:
		//Flags.  Can't be in a struct because it is not byte-aligned and it would screw up the packing.
		int         flags_unused1 :  1; //Can be reserved for OS use
		int         flags_unused2 :  1; //unused (at least on x86).  Initialized to 0.
		bool           flags_size :  1; //0 is 16-bit protected mode, 1 is 32-bit protected mode
		bool    flags_granularity :  1; //If 0 the limit is in 1B blocks (byte granularity), if 1 the limit is in 4KiB blocks (page granularity).
	private:
		uint32_t       _base_high :  8; //The upper 8 bits of the base.

	public:
		void set_limit(uint32_t limit) {
			_limit_low  =  limit & 0x0FFFF     ;
			_limit_high = (limit & 0xF0000)>>16;
		}
		inline uint32_t get_limit(void) const {
			return (_limit_high<<16) | _limit_low;
		}

		void set_base(uint32_t base) {
			_base_low  =  base & 0x00FFFFFF     ;
			_base_high = (base & 0xFF000000)>>24;
		}
		inline uint32_t get_base(void) const {
			return (_base_high<<24) | _base_low;
		}

		static void construct(EntryGDT* entry, uint32_t base,uint32_t limit, Access::AccessByte access) {
			entry->set_base(base);
			entry->set_limit(limit);

			entry->access.flags = access;

			entry->    flags_unused1 = 0;
			entry->    flags_unused2 = 0;
			entry->       flags_size = 1; //32-bit mode segment
			entry->flags_granularity = 1; //The limit is a 32-bit number and represents the number of 4KiB blocks
		}
} __attribute__((packed));
static_assert(sizeof(EntryGDT)==8,"Implementation error!");

#define MOSS_NUM_GDT 5
static EntryGDT _gdt_entries[MOSS_NUM_GDT];
void load_gdt(void) {
	EntryGDT::construct(_gdt_entries,   0x00000000u,         0u, EntryGDT::Access::AccessByte::          get_null()); //Null segment
	EntryGDT::construct(_gdt_entries+1, 0x00000000u,0xFFFFFFFFu, EntryGDT::Access::AccessByte::get_selector_code(0)); //Kernel code segment
	EntryGDT::construct(_gdt_entries+2, 0x00000000u,0xFFFFFFFFu, EntryGDT::Access::AccessByte::get_selector_data(0)); //Kernel data segment
	EntryGDT::construct(_gdt_entries+3, 0x00000000u,0xFFFFFFFFu, EntryGDT::Access::AccessByte::get_selector_code(3)); //User code segment
	EntryGDT::construct(_gdt_entries+4, 0x00000000u,0xFFFFFFFFu, EntryGDT::Access::AccessByte::get_selector_data(3)); //User data segment

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

	uint32_t base  = (uint32_t)(&_gdt_entries);         //The linear address of the first gdt_entry_t struct.
	uint16_t limit = sizeof(EntryGDT)*MOSS_NUM_GDT - 1; //The upper 16 bits of all selector limits.  Size of table minus 1.  sizeof(EntryGDT) should be 8.
	_lgdt(base,limit);
}
#undef MOSS_NUM_GDT


}}
