#pragma once

#include "../stdafx.h"

//#include "kernel.h" //TODO: remove
//extern bool ENABLE_LOG;



/*
Serial IO out of the CPU's IO ports.

What the ports mean, at least on Bochs:
	https://bochs.sourceforge.io/techspec/PORTS.LST
Example of `in` / `out`:
	https://wiki.osdev.org/Inline_Assembly/Examples#I.2FO_access
See also:
	https://www.felixcloutier.com/x86/in
	https://www.felixcloutier.com/x86/out
*/

__attribute__(( no_caller_saved_registers ))
MOSS_ND_INLINE uint8_t  x86_io_in8 ( uint16_t port )
{
	//if (ENABLE_LOG) klogf( "[R %04x->", (int)port );
	uint8_t val;
	asm volatile( "in   %0, %1\n" : "=a"(val) : "Nd"(port) : );
	//if (ENABLE_LOG) klogf( "%02x]", (int)val );
	return val;
}
__attribute__(( no_caller_saved_registers ))
MOSS_ND_INLINE uint16_t x86_io_in16( uint16_t port )
{
	uint16_t val;
	asm volatile( "in   %0, %1\n" : "=a"(val) : "Nd"(port) : );
	return val;
}
__attribute__(( no_caller_saved_registers ))
MOSS_ND_INLINE uint32_t x86_io_in32( uint16_t port )
{
	uint32_t val;
	asm volatile( "in   %0, %1\n" : "=a"(val) : "Nd"(port) : );
	return val;
}

MOSS_INLINE void x86_io_out8 ( uint16_t port, uint8_t  val )
{
	//if (ENABLE_LOG) klogf( "[W %04x<-%02x]", (int)port, (int)val );
	asm volatile( "out   %0, %1\n" : : "Nd"(port), "a"(val) : );
}
__attribute__(( no_caller_saved_registers ))
MOSS_INLINE void x86_io_out16( uint16_t port, uint16_t val )
{
	asm volatile( "out   %0, %1\n" : : "Nd"(port), "a"(val) : );
}
__attribute__(( no_caller_saved_registers ))
MOSS_INLINE void x86_io_out32( uint16_t port, uint32_t val )
{
	asm volatile( "out   %0, %1\n" : : "Nd"(port), "a"(val) : );
}
__attribute__(( no_caller_saved_registers ))
MOSS_INLINE void x86_io_out_post( uint8_t code )
{
	// Output codes used for debugging POST.  Also, for delays and reliability hacks.
	// See https://stackoverflow.com/a/6796109
	// See https://forum.osdev.org/viewtopic.php?p=257289&sid=7fd7fb690efb1ecafc3ce780d4479ffc#p257289
	x86_io_out8( 0x80, code );
}



MOSS_INLINE int  x86_serial_logc ( char ch ) { x86_io_out8( 0x03f8, ch ); return ch; }
MOSS_INLINE void x86_serial_logs ( char const* str )
{
	while ( *str != '\0' ) x86_serial_logc( *(str++) );
}
int              x86_serial_vlogf( char const* fmt_cstr, va_list args );



// Interrupts

typedef uintptr_t x86flags;

MOSS_INLINE void x86_core_interrupts_mask  (void) { asm("cli"); }
MOSS_INLINE void x86_core_interrupts_unmask(void) { asm("sti"); }

MOSS_INLINE bool x86_core_interrupts_are_enabled(void)
{
	x86flags flags;
	asm( "pushf\n" "pop %0\n" : "=r"(flags) : : );
	return flags & (1<<9);
}

MOSS_ND_INLINE x86flags x86_core_interrupts_push_and_disable(void)
{
	// "the compiler requires the value of the stack pointer to be the same after an `asm` statement
	// as it was on entry to the statement.", so alas we have to return the state.
	x86flags state;
	#if   defined __MOSS_32__
		asm( "pushfd\n" "cli\n" "pop %0\n" : "=r"(state) : : );
	#elif defined __MOSS_64__
		asm( "pushfq\n" "cli\n" "pop %0\n" : "=r"(state) : : );
	#else
		#error
	#endif
	return state;
}
MOSS_INLINE void x86_core_interrupts_pop( x86flags old_flags )
{
	#if   defined __MOSS_32__
		asm( "push %0\n" "popfd\n" : : "r"(old_flags) : "cc" );
	#elif defined __MOSS_64__
		asm( "push %0\n" "popfq\n" : : "r"(old_flags) : "cc" );
	#else
		#error
	#endif
}



/*
Programmable Interrupt Controller(s) (PIC(s))

The lowest common denominator of translation of IRQs into interrupt-vectors is the (now emulated)
PIC, almost always set up in a primary/secondary pair configuration, preconfigured and set up at
boot time.  Don't bother with delays; not necessary on reasonable hardware.  TODO: portability tho

See also:
	"doc/" for information on PICs, IRQs, and interrupts
	https://wiki.osdev.org/8259_PIC
	http://forum.osdev.org/viewtopic.php?f=1&t=26875&p=224967#p224967
	http://pdos.csail.mit.edu/6.828/2005/readings/hardware/8259A.pdf (pg. 10 onwards)
*/
#define X86PIC1_IOCMD  0x0020
#define X86PIC1_IODATA 0x0021
#define X86PIC2_IOCMD  0x00a0
#define X86PIC2_IODATA 0x00a1
void x86_picpair_reinit( uint8_t intvec_offset1, uint8_t intvec_offset2 );
MOSS_INLINE void x86_picpair_mask_all  ()
{
	x86_io_out8( X86PIC1_IODATA, 0xff );
	x86_io_out8( X86PIC2_IODATA, 0xff );
}
MOSS_INLINE void x86_picpair_unmask_all()
{
	x86_io_out8( X86PIC1_IODATA, 0x00 );
	x86_io_out8( X86PIC2_IODATA, 0x00 );
}
void x86_picpair_mask      ( uint8_t irq );
void x86_picpair_unmask    ( uint8_t irq );



MOSS_INLINE void x86_enable_sse(void)
{
	//https://wiki.osdev.org/SSE
	asm(
		//SSE
		"mov      rax, cr0\n"
		"and       ax, 65531\n" // 0xFFFB, clear coprocessor emulation CR0.EM
		"or        ax, 0x2\n" //set coprocessor monitoring  CR0.MP
		"mov      cr0, rax\n"
		"mov      rax, cr4\n"
		"or        ax, 3<<9\n" //set CR4.OSFXSR and CR4.OSXMMEXCPT at the same time
		"mov      cr4, rax\n"
		//AVX
		/*"xor      rcx, rcx\n"
		"xgetbv\n" // Load XCR0 register
		"or       eax, 7\n" // Set AVX, SSE, X87 bits
		"xsetbv\n" // Save back to XCR0*/
		: : : "rax"
	);
}



//Fence (Stores)
//	All stores before this instruction are globally visible before all stores after it.  Not ordered
//	with respect to loads / load-fences.  Requires SSE.  See also:
//	https://www.felixcloutier.com/x86/sfence
MOSS_INLINE void x86_sfence() { asm( "sfence" : : : ); }

//Fence (Loads)
//	All instructions finish at a local level, and no instructions begin until after.  Note that this
//	means that load instructions must have loaded their data, but that store operations might not
//	yet be globally visible.  Requires SSE2.  See also: https://www.felixcloutier.com/x86/lfence
MOSS_INLINE void x86_lfence() { asm( "lfence" : : : ); }

//Fence (Memory)
//	All load / store operations before this instruction are globally visible before all load / store
//	operations after.  Requires SSE2.  See also: https://www.felixcloutier.com/x86/mfence
MOSS_INLINE void x86_mfence() { asm( "mfence" : : : ); }



//Read timestamp counter
//	Requires 'tsc' in `cpuid` leaf 1.  See also: https://www.felixcloutier.com/x86/rdtsc
//	Suggest using `x86_mfence()` and/or `x86_lfence()` before, and `x86_lfence()` after
MOSS_ND_INLINE Cycles x86_rdtsc_nofences()
{
	Cycles tick;
	//See also example: https://gcc.gnu.org/onlinedocs/gcc/extensions-to-the-c-language-family/how-to-use-inline-assembly-language-in-c-code.html#x86-family-config-i386-constraints-md
	#if   defined __MOSS_32__
		asm( "rdtsc" : "=A"(tick) : : );
	#elif defined __MOSS_64__
		asm(
			"rdtsc\n"
			// Combine outputs ourselves to prevent reordering past any later fences.  Also, since
			// `rdtsc` also clears upper 32 bits of `rax` and `rdx`, don't need to worry about that
			// (compilers worry about it).
			"shl   rdx, 32\n"
			"or    rax, rdx\n"
			: "=a"(tick)
			:
			: "rdx", "cc"
		);
	#else
		#error
	#endif
	return tick;
}



void x86_delay_cycles( Cycles approx_cycles );



#if 0

/*
Programmable Interval Timer⁽¹⁾

Base frequency is ≈1.193181 MHz⁽²⁾, typically accurate to ±1.73 s/day.  This is divided⁽³⁾ by three
separate 16-bit numbers, giving frequencies on three 'channels'.  Channel 0 is gated to IRQ 0, while
Channel 2 is gated to the speaker.  Channel 1 is useless.

⁽¹⁾ See also https://wiki.osdev.org/Pit                                                      __
⁽²⁾ Due to complicated reasons, TV in the USA had a chrominance carrier nominally at of 3.57954 MHz.
    Thus, 4⨯ this, ≈14.318182 MHz, was a popular oscillator frequency in hardware.  This is then
    divided by 3 (for CPU) or 4 (for CGA), and logical-ANDed to get ≈1.193181 MHz (sometimes seen
    with a trailing 2 instead of 1 due to rounding in the middle of these calculations).
⁽³⁾ A divisor of 0 is interpreted as 65536.
*/

constexpr uint32 PIT_FREQ_HZ = 1'193'181;

union PIT_Cmd final
{
	struct
	{
		uint8 use_bcd   : 1; //Use binary-coded decimal (sketchy support)
		uint8 operation : 3; //Operating mode; see https://wiki.osdev.org/Pit#Operating_Modes
		uint8 access    : 2; //Access mode; 1=lobyte, 2=hibyte, 3=both
		uint8 channel   : 2; //Channel 0, 1, or 2.  3 is for special read-back command
	};
	uint8 packed;

	constexpr PIT_Cmd( uint8 packed ) noexcept : packed(packed) {}
};
static_assert( sizeof(PIT_Cmd) == sizeof(uint8) );

inline uint8 pit_data0_in (           ) noexcept { return io_in <uint8>(0x40    ); }
inline void  pit_data0_out( uint8 val ) noexcept { return io_out<uint8>(0x40,val); }

inline uint8 pit_data1_in (           ) noexcept { return io_in <uint8>(0x41    ); }
inline void  pit_data1_out( uint8 val ) noexcept { return io_out<uint8>(0x41,val); }

//Used for speaker (see below)
inline uint8 pit_data2_in (           ) noexcept { return io_in <uint8>(0x42    ); }
inline void  pit_data2_out( uint8 val ) noexcept { return io_out<uint8>(0x42,val); }

inline void pit_ctrl_out( PIT_Cmd const& cmd ) noexcept { return io_out<uint8>(0x43,cmd.packed); }
//(no read)



/*
Speaker:
	https://wiki.osdev.org/PC_Speaker
	https://github.com/systemd/systemd/blob/main/src/boot/efi/util.c#L582
	https://github.com/fpmurphy/UEFI-Utilities-2018/blob/master/MyApps/Beep/Beep.c
*/

void speaker_set_freq( uint32 freq_hz ) noexcept;

inline void speaker_set_on () noexcept
{
	uint8 val = CPU::io_in<uint8>( 0x61 );
	val |= 0x03;
	CPU::io_out<uint8>( 0x61, val );
}
inline void speaker_set_off() noexcept
{
	uint8 val = CPU::io_in<uint8>( 0x61 );
	val &= ~0x03;
	CPU::io_out<uint8>( 0x61, val );
}

template< class FnDelayUs >
void speaker_emit_bell( FnDelayUs const& fn ) noexcept
{
	speaker_set_freq( 440 ); //A
	speaker_set_on();
	fn( 500'000 );
	speaker_set_freq( 370 ); //F# (369.9944)
	fn( 500'000 );
	speaker_set_off();
}








#endif
