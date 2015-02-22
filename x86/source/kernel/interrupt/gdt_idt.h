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
		//		discr_type bit: 1 is code/data selector, 0 is TSS, LDT, or Gate (but also says earlier the same as the below):
		//	See http://wiki.osdev.org/Global_Descriptor_Table:
		//		discr_type bit: 1 is code selector, 0 is data selector
		//	See http://wiki.osdev.org/Segmentation
		//		discr_type bit: 1 is code/data selector, 0 is system
		union Access {
			class AccessByte { public:
				bool     accessed :  1; //Initialized to 0; CPU sets when segment is accessed
				bool           rw :  1; //Writable bit for data selectors / Readable bit for code selectors
				bool     dir_conf :  1; //For data selectors, 0 the segment grows up, 1 the segment grows down.  For code selectors, 0 only executable by processes with exactly privilege, 1 lower is okay too
				bool   discr_type :  1; //See above
				bool       unused :  1; //Initialized to 1
				uint8_t privilege :  2; //Ring
				bool      present :  1; //Must be 1 for all valid selectors

				static AccessByte get_null(void);
				static AccessByte get_selector_datacode(uint8_t ring);
			} flags; //Will be packed since EntryGDT is packed
			uint8_t          byte :  8;
		} access;
	private:
		uint32_t       limit_high :  4; //The upper 4 bits of the limit
	public:
		//Flags.  Can't be in a struct because it is not byte-aligned and it would screw up the packing.
		int          flags_unused :  2; //(at least on x86)
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
//	When an interrupt fires, the entry is used to jump to the appropriate handler function.  The offset
//		and selector are the address of this function in the GDT or LDT.
//	See http://wiki.osdev.org/IDT
class EntryIDT {
	private:
		uint32_t   offset_low : 16; //See above
		uint32_t     selector : 16; //See above
	public:
		uint8_t        unused :  8; //Initialized to 0 (must be?)
		struct {
			int     gate_type :  4; //See link above
			bool storage_elem :  1; //0 for interrupt gates
			uint8_t privilege :  2; //Specifies which privilege level the calling descriptor should have at minimum.
			bool      present :  1; //0 for unused interrupts or paging
		} type_attr;
	private:
		uint32_t offset_high  : 16; //See above

	public:
		void set_offset(uint32_t offset);
		uint32_t get_offset(void) const;

		static void construct(EntryIDT* entry, uint32_t offset, int privilege);
} __attribute__((packed));


extern "C" void gdt_lgdt(uint32_t base, size_t limit); //ASM routine

void load_gdt(void);




#if 0
#define MOSS_INTERRUPT(MACRO)\
	MACRO(  0) MACRO(  1) MACRO(  2) MACRO(  3) MACRO(  4) MACRO(  5) MACRO(  6) MACRO(  7)   MACRO(  8) MACRO(  9) MACRO( 10) MACRO( 11) MACRO( 12) MACRO( 13) MACRO( 14) MACRO( 15)\
	MACRO( 16) MACRO( 17) MACRO( 18) MACRO( 19) MACRO( 20) MACRO( 21) MACRO( 22) MACRO( 23)   MACRO( 24) MACRO( 25) MACRO( 26) MACRO( 27) MACRO( 28) MACRO( 29) MACRO( 30) MACRO( 31)\
	MACRO( 32) MACRO( 33) MACRO( 34) MACRO( 35) MACRO( 36) MACRO( 37) MACRO( 38) MACRO( 39)   MACRO( 40) MACRO( 41) MACRO( 42) MACRO( 43) MACRO( 44) MACRO( 45) MACRO( 46) MACRO( 47)\
	MACRO( 48) MACRO( 49) MACRO( 50) MACRO( 51) MACRO( 52) MACRO( 53) MACRO( 54) MACRO( 55)   MACRO( 56) MACRO( 57) MACRO( 58) MACRO( 59) MACRO( 60) MACRO( 61) MACRO( 62) MACRO( 63)\
	MACRO( 64) MACRO( 65) MACRO( 66) MACRO( 67) MACRO( 68) MACRO( 69) MACRO( 70) MACRO( 71)   MACRO( 72) MACRO( 73) MACRO( 74) MACRO( 75) MACRO( 76) MACRO( 77) MACRO( 78) MACRO( 79)\
	MACRO( 80) MACRO( 81) MACRO( 82) MACRO( 83) MACRO( 84) MACRO( 85) MACRO( 86) MACRO( 87)   MACRO( 88) MACRO( 89) MACRO( 90) MACRO( 91) MACRO( 92) MACRO( 93) MACRO( 94) MACRO( 95)\
	MACRO( 96) MACRO( 97) MACRO( 98) MACRO( 99) MACRO(100) MACRO(101) MACRO(102) MACRO(103)   MACRO(104) MACRO(105) MACRO(106) MACRO(107) MACRO(108) MACRO(109) MACRO(110) MACRO(111)\
	MACRO(112) MACRO(113) MACRO(114) MACRO(115) MACRO(116) MACRO(117) MACRO(118) MACRO(119)   MACRO(120) MACRO(121) MACRO(122) MACRO(123) MACRO(124) MACRO(125) MACRO(126) MACRO(127)\
	MACRO(128) MACRO(129) MACRO(130) MACRO(131) MACRO(132) MACRO(133) MACRO(134) MACRO(135)   MACRO(136) MACRO(137) MACRO(138) MACRO(139) MACRO(140) MACRO(141) MACRO(142) MACRO(143)\
	MACRO(144) MACRO(145) MACRO(146) MACRO(147) MACRO(148) MACRO(149) MACRO(150) MACRO(151)   MACRO(152) MACRO(153) MACRO(154) MACRO(155) MACRO(156) MACRO(157) MACRO(158) MACRO(159)\
	MACRO(160) MACRO(161) MACRO(162) MACRO(163) MACRO(164) MACRO(165) MACRO(166) MACRO(167)   MACRO(168) MACRO(169) MACRO(170) MACRO(171) MACRO(172) MACRO(173) MACRO(174) MACRO(175)\
	MACRO(176) MACRO(177) MACRO(178) MACRO(179) MACRO(180) MACRO(181) MACRO(182) MACRO(183)   MACRO(184) MACRO(185) MACRO(186) MACRO(187) MACRO(188) MACRO(189) MACRO(190) MACRO(191)\
	MACRO(192) MACRO(193) MACRO(194) MACRO(195) MACRO(196) MACRO(197) MACRO(198) MACRO(199)   MACRO(200) MACRO(201) MACRO(202) MACRO(203) MACRO(204) MACRO(205) MACRO(206) MACRO(207)\
	MACRO(208) MACRO(209) MACRO(210) MACRO(211) MACRO(212) MACRO(213) MACRO(214) MACRO(215)   MACRO(216) MACRO(217) MACRO(218) MACRO(219) MACRO(220) MACRO(221) MACRO(222) MACRO(223)\
	MACRO(224) MACRO(225) MACRO(226) MACRO(227) MACRO(228) MACRO(229) MACRO(230) MACRO(231)   MACRO(232) MACRO(233) MACRO(234) MACRO(235) MACRO(236) MACRO(237) MACRO(238) MACRO(239)\
	MACRO(240) MACRO(241) MACRO(242) MACRO(243) MACRO(244) MACRO(245) MACRO(246) MACRO(247)   MACRO(248) MACRO(249) MACRO(250) MACRO(251) MACRO(252) MACRO(253) MACRO(254) MACRO(255)

/*#define ISR(N) extern "C" void isr##N(void);
	MOSS_INTERRUPT(ISR)
#undef ISR*/
extern "C" void isr(void);

extern "C" void idt_lidt(uint32_t base, size_t limit); //ASM routine

void load_idt(void);
#endif
extern "C" void load_idt(void);


extern "C" void reload_segments(void); //ASM routine


}}