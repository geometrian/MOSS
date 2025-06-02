#include "descr_table.h"

#include "../draw.h"
#include "../kernel.h"

#include "misc.h"
#include "ps2.h"



/*
Again, refer to the material on exceptions / interrupts in "doc/" to understand what this is all
about.

See also:
	https://wiki.osdev.org/Interrupt_Vector_Table       (16-bit)
	https://wiki.osdev.org/Interrupt_Descriptor_Table   (32- and 64-bit)

	https://wiki.osdev.org/Interrupts_Tutorial

	https://wiki.osdev.org/Interrupt_Service_Routines#Clang
	https://clang.llvm.org/docs/AttributeReference.html#interrupt-x86
*/



struct _Entry16_GateDescr
{
	uint16_t segment : 16;
	uint16_t offset  : 16;
};

struct _Entry32_GateDescr
{
	uint64_t offset_0_15      : 16;
	uint64_t segment_selector : 16;
	uint64_t                  :  8; //reserved
	uint64_t type             :  4;
	uint64_t                  :  1; //zero
	uint64_t privilege        :  2;
	uint64_t is_present       :  1;
	uint64_t offset_16_31     : 16;
};

struct _Entry64_GateDescr
{
	uint64_t offset_0_15      : 16;
	uint64_t segment_selector : 16;
	uint64_t ist_offset       :  3;
	uint64_t                  :  5; //reserved
	uint64_t type             :  4;
	uint64_t zero             :  1;
	uint64_t privilege        :  2;
	uint64_t is_present       :  1;
	uint64_t offset_16_31     : 16;
	uint64_t offset_32_63     : 32;
	uint64_t                  : 32; //reserved
};

static_assert( sizeof(struct _Entry16_GateDescr) == 1*sizeof(uint32_t) );
static_assert( sizeof(struct _Entry32_GateDescr) == 2*sizeof(uint32_t) );
static_assert( sizeof(struct _Entry64_GateDescr) == 4*sizeof(uint32_t) );



struct InterruptFrame
{
	uint64_t rip;
	uint64_t cs;
	uint64_t rflags;
	uint64_t rsp;
	uint64_t ss; // Present only if privilege level / stack change
};

__attribute__(( format(printf,1,2), noreturn, no_caller_saved_registers ))
MOSS_INLINE static void _vec_kpanic( char const* fmt_cstr, ... )
{
	#ifdef __MOSS_CLANG__
		#pragma clang diagnostic push
		#pragma clang diagnostic ignored "-Winterrupt-service-routine"
	#endif
	va_list args;
	va_start( args, fmt_cstr );
	kernel_panic_va( fmt_cstr, args );
	va_end( args );
	#ifdef __MOSS_CLANG__
		#pragma clang diagnostic pop
	#endif
}
__attribute__(( noreturn, no_caller_saved_registers ))
static void _vec_forbidden( int vec, char const* name )
{
	#ifdef __MOSS_CLANG__
		#pragma clang diagnostic push
		#pragma clang diagnostic ignored "-Winterrupt-service-routine"
	#endif
	kernel_panic( "Processor received forbidden interrupt-vector %d (%s)!", vec,name );
	#ifdef __MOSS_CLANG__
		#pragma clang diagnostic pop
	#endif
}

// (reserved) | N/A | no error code
__attribute__(( interrupt ))
static void _vec_reserved( struct InterruptFrame* tmp_info )
{
	_vec_kpanic("Processor received unknown (reserved) interrupt-vector!");
}

// Hardware interrupt from PIC
__attribute__(( no_caller_saved_registers ))
static void _vec_pic( uint8_t vec )
{
	uint8_t irq = vec - 32;

	// https://wiki.osdev.org/Interrupts#General_IBM-PC_Compatible_Interrupt_Information
	switch ( irq )
	{

		// Programmable interrupt timer (PIT)
		case  0:
			break;

		// PS/2 Keyboard
		case  1:
			x86_ps2_handle_irq( irq );
			break;

		// Used internally for PIC cascade (never raised)
		case  2:
			_vec_forbidden( vec, "IRQ 2" );

		// Serial COM2
		case  3: [[fallthrough]];

		// Serial COM1
		case  4: [[fallthrough]];

		// LPT2
		case  5: [[fallthrough]];

		// Floppy disk
		case  6:
			_vec_forbidden( vec, "Unimplemented IRQ" );

		// LPT1 or (more usually) spurious interrupt
		case  7:
			// Check PIC 1 ISR to see if this was spurious
			if ( vec == 7 )
			{
				x86_io_out8( X86PIC1_IOCMD, 0x0b );
				if (!( vec & x86_io_in8( X86PIC1_IOCMD ) )) return;
			}
			_vec_forbidden( vec, "Unimplemented IRQ" );

		// CMOS real-time clock
		case  8: [[fallthrough]];

		// Free for peripherals / legacy SCSI / NIC
		case  9: [[fallthrough]];

		// Free for peripherals / SCSI / NIC
		case 10: [[fallthrough]];

		// Free for peripherals / SCSI / NIC
		case 11: [[fallthrough]];

		// PS/2 Mouse
		case 12:
			x86_ps2_handle_irq( irq );
			break;

		// FPU / Coprocessor / Inter-processor
		case 13: [[fallthrough]];

		// Primary ATA Hard Disk
		case 14:
			_vec_forbidden( vec, "Unimplemented IRQ" );

		// Secondary ATA Hard Disk or spurious
		case 15:
			// Check PIC 2 ISR to see if this was spurious
			if ( vec == 15 )
			{
				x86_io_out8( X86PIC2_IOCMD, 0x0b );
				if (!( vec & x86_io_in8( X86PIC2_IOCMD ) ))
				{
					x86_io_out8( X86PIC1_IOCMD, 0x20 ); // Still need to issue EOI to primary PIC
					return;
				}
			}
			_vec_forbidden( vec, "Unimplemented IRQ" );

		__moss_switch_default_unnecessary;
	}

	if ( irq < 8 )
	{
		// End of interrupt
		x86_io_out8( X86PIC1_IOCMD, 0x20 );
	}
	else
	{
		// End of interrupt; must issue to both PICs
		x86_io_out8( X86PIC2_IOCMD, 0x20 );
		x86_io_out8( X86PIC1_IOCMD, 0x20 );
	}
}

// integer divide-by-zero | fault | no error code
__attribute__(( interrupt ))
static void _vec0_div_err( struct InterruptFrame* tmp_info )
{
	#ifdef __MOSS_CLANG__
		#pragma clang diagnostic push
		#pragma clang diagnostic ignored "-Winterrupt-service-routine"
	#endif
	draw_rect_color( &g_kernel.grfx, sRGB_A_(0,255,0,255), Rect_(50,50,300,200), 5 );
	#ifdef __MOSS_CLANG__
		#pragma clang diagnostic pop
	#endif

	//core_halt();
	asm(
		"cli\n"
		"hlt\n"
	);
}
// debug | fault/trap | no error code
__attribute__(( interrupt ))
static void _vec1_debug( struct InterruptFrame* tmp_info )
{
	_vec_kpanic( "Processor received unimplemented interrupt-vector %d (%s)!", 1,"debug" );
}
// non-maskable interrupt | interrupt | no error code
__attribute__(( interrupt ))
static void _vec2_nmi( struct InterruptFrame* tmp_info )
{
	// https://wiki.osdev.org/Non_Maskable_Interrupt
	_vec_kpanic( "Processor received %s, indicating hardware error!", "non-maskable interrupt" );
}
// breakpoint | trap | no error code
__attribute__(( interrupt ))
static void _vec3_breakpoint( struct InterruptFrame* tmp_info )
{
	_vec_kpanic( "Processor received unimplemented interrupt-vector %d (%s)!", 3,"breakpoint" );
}
 // throw exception if overflow flag | trap | no error code
__attribute__(( interrupt ))
static void _vec4_overflow( struct InterruptFrame* tmp_info )
{
	_vec_forbidden( 4, "overflow" );
}
// throw exception if range exceeded | fault | no error code
__attribute__(( interrupt ))
static void _vec5_bound_exceeded( struct InterruptFrame* tmp_info )
{
	_vec_forbidden( 5, "bound exceeded" );
}
// invalid opcode | fault | no error code
__attribute__(( interrupt ))
static void _vec6_invalid_opcode( struct InterruptFrame* tmp_info )
{
	_vec_forbidden( 6, "invalid opcode" );
}
// FPU disabled or nonexistent | fault | no error code
__attribute__(( interrupt ))
static void _vec7_fpu_unavail( struct InterruptFrame* tmp_info )
{
	_vec_kpanic( "Processor received unimplemented interrupt-vector %d (%s)!", 7,"FPU unavailable" );
}
// double fault | abort | error code = 0
__attribute__(( interrupt ))
static void _vec8_double_fault( struct InterruptFrame* tmp_info, uint64_t error_code_0 )
{
	_vec_forbidden( 8, "double-fault" );
}
// external FPU segment violation | fault | no error code
__attribute__(( interrupt ))
static void _vec9_ext_fpu_segfault( struct InterruptFrame* tmp_info )
{
	_vec_forbidden( 9, "external FPU segfault" );
}
// invalid tss segment | fault | error code
__attribute__(( interrupt ))
static void _vec10_invalid_tss_seg( struct InterruptFrame* tmp_info, uint64_t error_code )
{
	_vec_kpanic( "Processor received unimplemented interrupt-vector %d (%s)!", 10,"invalid TSS segment" );
}
// segment not present | fault | error code
__attribute__(( interrupt ))
static void _vec11_seg_not_pres( struct InterruptFrame* tmp_info, uint64_t error_code )
{
	_vec_kpanic( "Processor received unimplemented interrupt-vector %d (%s)!", 11,"segment not present" );
}
// stack-segment fault | fault | error code
__attribute__(( interrupt ))
static void _vec12_stack_seg( struct InterruptFrame* tmp_info, uint64_t error_code )
{
	_vec_kpanic( "Processor received unimplemented interrupt-vector %d (%s)!", 12,"stack segment" );
}
// general protection fault | fault | error code
__attribute__(( interrupt ))
static void _vec13_gen_protect( struct InterruptFrame* tmp_info, uint64_t error_code )
{
	_vec_forbidden( 13, "general protection" );
}
// page fault | fault | error code
__attribute__(( interrupt ))
static void _vec14_page_fault( struct InterruptFrame* tmp_info, uint64_t error_code )
{
	_vec_kpanic( "Processor received unimplemented interrupt-vector %d (%s)!", 14,"page fault" );
}
// (15 reserved)
// x87 FPU exception | fault | no error code
__attribute__(( interrupt ))
static void _vec16_x87_exception( struct InterruptFrame* tmp_info )
{
	_vec_forbidden( 16, "x87 exception" );
}
// alignment check failed | fault | error code
__attribute__(( interrupt ))
static void _vec17_align_check( struct InterruptFrame* tmp_info, uint64_t error_code )
{
	_vec_forbidden( 17, "alignment check" );
}
// model-specific machine check enabled and failed | abort | no error code
__attribute__(( interrupt ))
static void _vec18_machine_check( struct InterruptFrame* tmp_info )
{
	_vec_kpanic( "Processor received %s, indicating hardware error!", "machine check exception" );
}
// SSE floating-point exception | fault | no error code
__attribute__(( interrupt ))
static void _vec19_sse_exception( struct InterruptFrame* tmp_info )
{
	_vec_forbidden( 19, "SSE exception" );
}
//  | fault | no error code
__attribute__(( interrupt ))
static void _vec20_virt_except( struct InterruptFrame* tmp_info )
{
	_vec_kpanic( "Processor received unimplemented interrupt-vector %d (%s)!", 30,"virtualization exception" );
}
//  | fault | error code
__attribute__(( interrupt ))
static void _vec21_ctrl_protect( struct InterruptFrame* tmp_info, uint64_t error_code )
{
	_vec_kpanic( "Processor received unimplemented interrupt-vector %d (%s)!", 21,"control protection exception" );
}
// (22--27 reserved)
//  | fault | no error code
__attribute__(( interrupt ))
static void _vec28_hyper_inject( struct InterruptFrame* tmp_info )
{
	_vec_kpanic( "Processor received unimplemented interrupt-vector %d (%s)!", 28,"hypervisor injection exception" );
}
//  | fault | error code
__attribute__(( interrupt ))
static void _vec29_vmm_comm( struct InterruptFrame* tmp_info, uint64_t error_code )
{
	_vec_kpanic( "Processor received unimplemented interrupt-vector %d (%s)!", 29,"VMM communication exception" );
}
// security exception | fault | error code
__attribute__(( interrupt ))
static void _vec30_security( struct InterruptFrame* tmp_info, uint64_t error_code )
{
	_vec_kpanic( "Processor received unimplemented interrupt-vector %d (%s)!", 30,"security exception" );
}
// (31 reserved)
// hardware interrupt 32--47 (PIC)
__attribute__(( interrupt ))
static void _vec32_pic( struct InterruptFrame* tmp_info ) { _vec_pic(32); }
__attribute__(( interrupt ))
static void _vec33_pic( struct InterruptFrame* tmp_info ) { _vec_pic(33); }
__attribute__(( interrupt ))
static void _vec34_pic( struct InterruptFrame* tmp_info ) { _vec_pic(34); }
__attribute__(( interrupt ))
static void _vec35_pic( struct InterruptFrame* tmp_info ) { _vec_pic(35); }
__attribute__(( interrupt ))
static void _vec36_pic( struct InterruptFrame* tmp_info ) { _vec_pic(36); }
__attribute__(( interrupt ))
static void _vec37_pic( struct InterruptFrame* tmp_info ) { _vec_pic(37); }
__attribute__(( interrupt ))
static void _vec38_pic( struct InterruptFrame* tmp_info ) { _vec_pic(38); }
__attribute__(( interrupt ))
static void _vec39_pic( struct InterruptFrame* tmp_info ) { _vec_pic(39); }
__attribute__(( interrupt ))
static void _vec40_pic( struct InterruptFrame* tmp_info ) { _vec_pic(40); }
__attribute__(( interrupt ))
static void _vec41_pic( struct InterruptFrame* tmp_info ) { _vec_pic(41); }
__attribute__(( interrupt ))
static void _vec42_pic( struct InterruptFrame* tmp_info ) { _vec_pic(42); }
__attribute__(( interrupt ))
static void _vec43_pic( struct InterruptFrame* tmp_info ) { _vec_pic(43); }
__attribute__(( interrupt ))
static void _vec44_pic( struct InterruptFrame* tmp_info ) { _vec_pic(44); }
__attribute__(( interrupt ))
static void _vec45_pic( struct InterruptFrame* tmp_info ) { _vec_pic(45); }
__attribute__(( interrupt ))
static void _vec46_pic( struct InterruptFrame* tmp_info ) { _vec_pic(46); }
__attribute__(( interrupt ))
static void _vec47_pic( struct InterruptFrame* tmp_info ) { _vec_pic(47); }

/*//  |  |
__attribute__(( interrupt ))
static void _vec_( struct InterruptFrame* tmp_info )
{
}*/



#define _DEF_GATE_HELPER( FNPTR, CLI_0or1, PRESENT_0or1 )\
	(struct _Entry64_GateDescr){\
		(uint64_t)FNPTR,\
		0x0008, /* kernel code in GDT */\
		0, /* No IST */\
		/* 0, (reserved) */\
		0xE | CLI_0or1, /* trap gate or interrupt gate, differing by whether they clear interrupts */\
		0, /* zero */\
		3, /* privilege level */\
		PRESENT_0or1, /* present */\
		(uint64_t)FNPTR >> 16,\
		(uint64_t)FNPTR >> 32\
		/*, 0 (reserved) */\
	}
#define DEF_GATE_NOTIMPL()\
	_DEF_GATE_HELPER( NULL, 0, 0 )
#define DEF_GATE( FNPTR, CLI_0or1 )\
	_DEF_GATE_HELPER( FNPTR, CLI_0or1, 1 )

#define NUM_IDT64 48 // could be up to 256
/*
Quoth Intel Vol. 3A §6.10:
	"The base addresses of the IDT should be aligned on an 8-byte boundary to maximize performance
	of cache line fills."
That's already accomplished by `uint64_t`.  Let's align it 16, just because that's the size of the
each descriptor.
*/
alignas(16) static struct _Entry64_GateDescr _idt64[ NUM_IDT64 ];



// https://wiki.osdev.org/Exception
// https://wiki.osdev.org/Interrupts_Tutorial

void x86_core_setup_idt64(void)
{
	_idt64[ 0] = DEF_GATE( &_vec0_div_err         , 0 );
	_idt64[ 1] = DEF_GATE( &_vec1_debug           , 0 );
	_idt64[ 2] = DEF_GATE( &_vec2_nmi             , 0 );
	_idt64[ 3] = DEF_GATE( &_vec3_breakpoint      , 0 );
	_idt64[ 4] = DEF_GATE( &_vec4_overflow        , 0 );
	_idt64[ 5] = DEF_GATE( &_vec5_bound_exceeded  , 0 );
	_idt64[ 6] = DEF_GATE( &_vec6_invalid_opcode  , 0 );
	_idt64[ 7] = DEF_GATE( &_vec7_fpu_unavail     , 0 );
	_idt64[ 8] = DEF_GATE( &_vec8_double_fault    , 0 );
	_idt64[ 9] = DEF_GATE( &_vec9_ext_fpu_segfault, 0 );
	_idt64[10] = DEF_GATE( &_vec10_invalid_tss_seg, 0 );
	_idt64[11] = DEF_GATE( &_vec11_seg_not_pres   , 0 );
	_idt64[12] = DEF_GATE( &_vec12_stack_seg      , 0 );
	_idt64[13] = DEF_GATE( &_vec13_gen_protect    , 0 );
	_idt64[14] = DEF_GATE( &_vec14_page_fault     , 0 );
	_idt64[15] = DEF_GATE( &_vec_reserved         , 0 );
	_idt64[16] = DEF_GATE( &_vec16_x87_exception  , 0 );
	_idt64[17] = DEF_GATE( &_vec17_align_check    , 0 );
	_idt64[18] = DEF_GATE( &_vec18_machine_check  , 0 );
	_idt64[19] = DEF_GATE( &_vec19_sse_exception  , 0 );
	_idt64[20] = DEF_GATE( &_vec20_virt_except    , 0 );
	_idt64[21] = DEF_GATE( &_vec21_ctrl_protect   , 0 );
	_idt64[22] = DEF_GATE( &_vec_reserved         , 0 );
	_idt64[23] = DEF_GATE( &_vec_reserved         , 0 );
	_idt64[24] = DEF_GATE( &_vec_reserved         , 0 );
	_idt64[25] = DEF_GATE( &_vec_reserved         , 0 );
	_idt64[26] = DEF_GATE( &_vec_reserved         , 0 );
	_idt64[27] = DEF_GATE( &_vec_reserved         , 0 );
	_idt64[28] = DEF_GATE( &_vec28_hyper_inject   , 0 );
	_idt64[29] = DEF_GATE( &_vec29_vmm_comm       , 0 );
	_idt64[30] = DEF_GATE( &_vec30_security       , 0 );
	_idt64[31] = DEF_GATE( &_vec_reserved         , 0 );

	_idt64[32] = DEF_GATE( &_vec32_pic            , 1 );
	_idt64[33] = DEF_GATE( &_vec33_pic            , 1 );
	_idt64[34] = DEF_GATE( &_vec34_pic            , 1 );
	_idt64[35] = DEF_GATE( &_vec35_pic            , 1 );
	_idt64[36] = DEF_GATE( &_vec36_pic            , 1 );
	_idt64[37] = DEF_GATE( &_vec37_pic            , 1 );
	_idt64[38] = DEF_GATE( &_vec38_pic            , 1 );
	_idt64[39] = DEF_GATE( &_vec39_pic            , 1 );
	_idt64[40] = DEF_GATE( &_vec40_pic            , 1 );
	_idt64[41] = DEF_GATE( &_vec41_pic            , 1 );
	_idt64[42] = DEF_GATE( &_vec42_pic            , 1 );
	_idt64[43] = DEF_GATE( &_vec43_pic            , 1 );
	_idt64[44] = DEF_GATE( &_vec44_pic            , 1 );
	_idt64[45] = DEF_GATE( &_vec45_pic            , 1 );
	_idt64[46] = DEF_GATE( &_vec46_pic            , 1 );
	_idt64[47] = DEF_GATE( &_vec47_pic            , 1 );

	union _DescrTable64_Info info;
	info.base_addr = (uint64_t)&_idt64;
	info.limit = sizeof(struct _Entry64_GateDescr)*NUM_IDT64 - 1;

	asm(
		"lidt %0\n"
		:
		:
		"m"(info)
	);
}



#if 0

struct interrupt_frame
{
	uword_t ip;
	uword_t cs;
	uword_t flags;
	uword_t sp;
	uword_t ss;
};

//Interrupt and exception handlers should only call functions with the ‘no_caller_saved_registers’
//attribute, or should be compiled with the ‘-mgeneral-regs-only’ flag to avoid saving unused non-GPR registers.

__attribute__(( interrupt ))
void interrupt_handler( struct interrupt_frame* frame )
{
}
__attribute__ ((interrupt))
void exception_handler( struct stack_frame* frame, unsigned long code )
{

}
#endif
