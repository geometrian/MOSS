#pragma once
#include "../../includes.h"


namespace MOSS { namespace Interrupts {


//GDT Entry
class EntryGDT {
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
			class AccessByte { public:
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

//IDT Entry or "gate".  Types can be interrupt gate, task gate, trap gate
//	When an interrupt fires, the entry is used to jump to the appropriate handler function.  For interrupt/trap gates, the offset
//		and selector are the address of this function in the GDT or LDT.  Specifically, the selector should refer to the code
//		segment where the handler function resides in the GDT, and the offset is where that function is within.  For task gates,
//		the offset is not used and the selector is the TSS selector.
//	See http://www.brokenthorn.com/Resources/OSDev15.html
//	See http://wiki.osdev.org/IDT
class EntryIDT {
	private:
		uint32_t       offset_low : 16; //See above
		uint16_t         selector : 16; //See above
	public:
		uint8_t           unused1 :  5; //Not used
		uint8_t           unused2 :  3; //Reserved for interrupt/trap gates and must be 0, unused for task gates.
		union Type {
			struct TypeByte {
				//Of the format 0bX110, where X determines size
				//According to BrokenThorn, the first five bits here are one field, but according to OSDev, they're separate.  OSDev says
				//	they must be 0 for interrupt gates, but doesn't say anything about other cases.  I'm leaving the fifth bit separate
				//	but leaving it zero as per BrokenThorn's direction.
				enum GateType { //Be sure to update code for storage_elem if change this!  See also http://www.acm.uiuc.edu/sigops/roll_your_own/i386/systbl.html#SystemDesc
					Task32      = (uint8_t)(0x5u),
					Interrupt16 = (uint8_t)(0x6u),
					Trap16      = (uint8_t)(0x7u),
					Interrupt32 = (uint8_t)(0xEu),
					Trap32      = (uint8_t)(0xFu),
				};// gate_type :  4; //See link above
				uint8_t gate_type :  4; //TODO: this can't be of type GateType because of some weird alignment issue.
				bool storage_elem :  1; //Initialized to 0
				uint8_t privilege :  2; //Specifies which privilege level the calling descriptor should at least have.
				bool      present :  1; //1=present, 0=not; needs to be set to work; 0 for unused interrupts or paging
			} flags;
			uint8_t          byte :  8;
		} type;
	private:
		uint32_t offset_high  : 16; //See above

	public:
		void set_offset(uint32_t offset);
		uint32_t get_offset(void) const;

		static void construct(EntryIDT* entry, uint32_t offset, Type::TypeByte::GateType type, int privilege);
} __attribute__((packed));


extern "C" void gdt_lgdt(uint32_t base, size_t limit); //ASM routine

//Note that this function ought to be called with interrupts disabled; if an interrupt fires while in progress . . .
void load_gdt(void);


extern "C" void idt_lidt(uint32_t base, size_t limit); //ASM routine

//Note that this function ought to be called with interrupts disabled; if an interrupt fires while in progress . . .
void load_idt(void);


extern "C" void reload_segments(void); //ASM routine


}}