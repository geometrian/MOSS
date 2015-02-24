#include "idt.h"


namespace MOSS { namespace Interrupts {


//IDT Entry or "gate".  Types can be interrupt gate, task gate, trap gate.
//	The key difference between an interrupt gate an a trap gate is that interrupt gates disable interrupts before processing.
//	When an interrupt fires, the entry is used to jump to the appropriate handler function.  For interrupt/trap gates, the offset
//		and selector are the address of this function in the GDT or LDT.  Specifically, the selector should refer to the code
//		segment where the handler function resides in the GDT, and the offset is where that function is within.  For task gates,
//		the offset is not used and the selector is the TSS selector.
//	See http://www.brokenthorn.com/Resources/OSDev15.html
//	See http://wiki.osdev.org/IDT
class EntryIDT final {
	private:
		uint32_t      _offset_low : 16; //See above
		uint16_t        _selector : 16; //See above
	public:
		uint8_t           unused1 :  5; //Not used
		uint8_t           unused2 :  3; //Reserved for interrupt/trap gates and must be 0, unused for task gates.
		union Type {
			struct TypeByte final {
				//Of the format 0bX110, where X determines size
				//According to BrokenThorn, the first five bits here are one field, but according to OSDev, they're separate.  OSDev says
				//	they must be 0 for interrupt gates, but doesn't say anything about other cases.  I'm leaving the fifth bit separate
				//	but leaving it zero as per BrokenThorn's direction.
				enum GateType { //Be sure to update code for storage_elem if change this!  See also http://www.acm.uiuc.edu/sigops/roll_your_own/i386/systbl.html#SystemDesc
					Task32      = static_cast<uint8_t>(0x05u),
					Interrupt16 = static_cast<uint8_t>(0x06u),
					Trap16      = static_cast<uint8_t>(0x07u),
					Interrupt32 = static_cast<uint8_t>(0x0Eu),
					Trap32      = static_cast<uint8_t>(0x0Fu),
				};// gate_type :  4; //See link above
				uint8_t gate_type :  4; //TODO: this can't be of type GateType because of some weird alignment issue.
				bool storage_elem :  1; //Initialized to 0
				uint8_t privilege :  2; //Specifies which privilege level the calling descriptor should at least have.
				bool      present :  1; //1=present, 0=not; needs to be set to work; 0 for unused interrupts or paging
			} flags;
			uint8_t          byte :  8;
		} type;
	private:
		uint32_t    _offset_high  : 16; //See above

	public:
		void set_offset(uint32_t offset) {
			_offset_low  =  offset&0x0000FFFF       ;
			_offset_high = (offset&0xFFFF0000) >> 16;
		}
		inline uint32_t get_offset(void) const {
			return (_offset_high<<16) | _offset_low;
		}

		static void construct(EntryIDT* entry, uint32_t offset, Type::TypeByte::GateType type, int privilege) {
			entry->set_offset(offset);
			entry->_selector = 0x0008; //Location of kernel code segment in GDT

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
} __attribute__((packed));
static_assert(sizeof(EntryIDT)==8,"EntryIDT is the wrong size!");


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
#define ISR_ASM(N) extern "C" void isr##N##_asm(void);
MOSS_INTERRUPT(ISR_ASM)
#undef ISR_ASM

static EntryIDT idt_entries[256];
void load_idt(void) {
	uint32_t base  = (uint32_t)(&idt_entries);
	uint16_t limit = sizeof(EntryIDT)*256 - 1;

	#define IDT_SET_GATE(N) EntryIDT::construct(idt_entries+N, (uint32_t)(isr##N##_asm), EntryIDT::Type::TypeByte::Interrupt32, 0);
	MOSS_INTERRUPT(IDT_SET_GATE)
	#undef IDT_SET_GATE

	idt_lidt(base,limit);
}


}}
