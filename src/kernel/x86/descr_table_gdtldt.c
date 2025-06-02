#include "descr_table.h"



/*
Again, refer to the material on GDT / LDT / memory segmentation in "doc/" to understand what this is
all about.
*/



struct _Entry16_SegDescr
{
	uint64_t limit       : 16;
	uint64_t base        : 24;
	uint64_t access_byte :  8;
	uint64_t             :  8;
};

struct _Entry3264_SegDescr
{
	uint64_t limit_0_15  : 16;
	uint64_t base_0_23   : 24;
	uint64_t access_byte :  8;
	uint64_t limit_16_19 :  4;
	uint64_t flags       :  4;
	uint64_t base_24_31  :  8;
};

static_assert( sizeof(struct _Entry16_SegDescr) == 2*sizeof(uint32_t) );
static_assert( sizeof(struct _Entry3264_SegDescr) == 2*sizeof(uint32_t) );



#define NUM_GDT64 5
/*
Quoth Intel Vol. 3A §3.5.1:
	"The base address of the GDT should be aligned on an eight-byte boundary to yield the best
	processor performance."
That's already accomplished by `uint64_t`.
*/
alignas(8) static struct _Entry3264_SegDescr _gdt64[ NUM_GDT64 ] =
{
	//https://wiki.osdev.org/GDT_Tutorial#What_to_Put_In_a_GDT
	//                            limit   base   access limit flag base
	//                            0–15    0–23    byte  16–19      24–31
	(struct _Entry3264_SegDescr){ 0xFFFF, 0x00000, 0x00, 0xF, 0x0, 0x00 }, //null descriptor
	(struct _Entry3264_SegDescr){ 0xFFFF, 0x00000, 0x9a, 0xF, 0xa, 0x00 }, //kernel code
	(struct _Entry3264_SegDescr){ 0xFFFF, 0x00000, 0x92, 0xF, 0xc, 0x00 }, //kernel data
	(struct _Entry3264_SegDescr){ 0xFFFF, 0x00000, 0xfa, 0xF, 0xa, 0x00 }, //user code
	(struct _Entry3264_SegDescr){ 0xFFFF, 0x00000, 0xf2, 0xF, 0xc, 0x00 }  //user data
};



// See ".asm" file.

void _core_lgdt32( uint32_t addr );

void _core_lgdt64( uint64_t addr );



void x86_core_longmode64_setup_gdt64(void)
{
	union _DescrTable64_Info info;
	info.base_addr = (uint64_t)&_gdt64;
	info.limit = sizeof(struct _Entry3264_SegDescr)*NUM_GDT64 - 1;

	_core_lgdt64( (uint64_t)&info );
}
