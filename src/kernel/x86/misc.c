#include "misc.h"



bool ENABLE_LOG = false;



static int _write_ch_serial( int ch, void* user_ptr )
{
	return x86_serial_logc( ch );
}
int x86_serial_vlogf( char const* fmt_cstr, va_list args )
{
	FILE file_fn = __moss_file_fn( &_write_ch_serial, NULL );
	return vfprintf( &file_fn, fmt_cstr,args );
}



void x86_picpair_reinit( uint8_t intvec_offset1, uint8_t intvec_offset2 )
{
	// See the surprisingly readable datasheet, pg. 10 onwards
	// http://pdos.csail.mit.edu/6.828/2005/readings/hardware/8259A.pdf

	// 1st control word: will have 4th control word, cascade mode, call interval 8, edge-triggered
	x86_io_out8( X86PIC1_IOCMD, 0x11 );
	x86_io_out8( X86PIC2_IOCMD, 0x11 );

	// 2nd control word: interrupt vector offsets
	x86_io_out8( X86PIC1_IODATA, intvec_offset1 );
	x86_io_out8( X86PIC2_IODATA, intvec_offset2 );

	// 3rd control word: tell primary there is secondary at input bitset { 2 }
	x86_io_out8( X86PIC1_IODATA, 1<<2 );
	// 3rd control word: tell secondary that it is second in cascade
	x86_io_out8( X86PIC2_IODATA, 2    );

	// 4th control word: use 8086 mode, normal end-of-interrupt, non-buffered, not special mode
	x86_io_out8( X86PIC1_IODATA, 1 );
	x86_io_out8( X86PIC2_IODATA, 1 );

	// if we were doing delays, a delay would only be necessary here, since interleaving commands
	// already is a delay
}
void x86_picpair_mask      ( uint8_t irq )
{
	if ( irq < 8 )
	{
		x86_io_out8( X86PIC1_IODATA, x86_io_in8(X86PIC1_IODATA) | (1<<irq) );
	}
	else
	{
		irq -= 8;
		x86_io_out8( X86PIC2_IODATA, x86_io_in8(X86PIC2_IODATA) | (1<<irq) );
	}
}
void x86_picpair_unmask    ( uint8_t irq )
{
	if ( irq < 8 )
	{
		x86_io_out8( X86PIC1_IODATA, x86_io_in8(X86PIC1_IODATA) | ~(1<<irq) );
	}
	else
	{
		irq -= 8;
		x86_io_out8( X86PIC2_IODATA, x86_io_in8(X86PIC2_IODATA) | ~(1<<irq) );
	}
}



void x86_delay_cycles( Cycles approx_cycles )
{
	Cycles start = x86_rdtsc_nofences();
	Cycles end = start + approx_cycles;
	Cycles now;

	LOOP:
		now = x86_rdtsc_nofences();
		if ( now <= end ) goto LOOP;
}



#if 0

void speaker_set_freq( uint32_t freq_hz )
{
	pit_ctrl_out({ 0b10'11'011'0 }); // channel 2, two bytes, square-wave, binary

	// Frequency divisor.  E.g. if `freq_hz`=440, then `rate_divisor`=2711, because you divide the
	// 1.193181 MHz signal by 2711 to get 440 Hz.  `rate_divisor` will be no larger than two bytes
	// when `freq_hz` is 19 Hz or greater; clamp it so we don't get artifacts.
	if ( freq_hz < 19 ) freq_hz=19;
	uint32 rate_divisor = PIT_FREQ_HZ / freq_hz;

	pit_data2_out( (uint8) rate_divisor     );
	pit_data2_out( (uint8)(rate_divisor>>8) );
}

#endif
