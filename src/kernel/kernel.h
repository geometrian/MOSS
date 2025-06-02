#pragma once

//#include "stdafx.h"

#include "device/inpdev.h"

#include "draw.h"
#include "text_ui.h"



typedef uint8_t* AddrPhys;
typedef uint8_t* AddrVirt;

typedef enum
{
	// Memory used for MOSS and application purposes
	ADDR_MOSSPOOL,
	
	// Reserved or unknown region
	ADDR_OTHER,
	// ACPI memory
	ADDR_ACPI,
	// MMIO addresses
	ADDR_MMIO
}
ADDR;

struct AddrRange
{
	ADDR type;
	AddrPhys start;
	uint64_t size;
};

struct AddrMap
{
	uint64_t mem_size;

	size_t           blocks_count;
	struct AddrRange blocks[ 1 /* actually as many as `.blocks_count` */ ];
};



struct MOSS_Kernel
{
	bool is_inited;

	atomic_flag mutex;

	struct
	{
		unsigned count;
	}
	panic;

	struct BitmapFont _font;
	struct TextUI _textui;
	struct Image _img_screen;
	struct Graphics grfx;

	struct AddrMap* addr_map;

	/*struct
	{
		uint8_t code[];
		uint8_t count;
	}
	keyboard_buffer;*/
};

extern struct MOSS_Kernel g_kernel;



void kernel_init(void);

void kernel_init_load_text(void);

void kernel_init_fbogrfx( COLOR color_type, vec2u res, uint32_t scanline_sz, void* data );

MOSS_INLINE void kernel_init_mmap( struct AddrMap* addr_map ) { g_kernel.addr_map=addr_map; }



int klogc( char ch );
int klogs( char const* str );
__attribute__(( format(printf,1,2) ))
int klogf ( char const* fmt_cstr,...          );
__attribute__(( format(printf,1,0) ))
int kvlogf( char const* fmt_cstr,va_list args );

MOSS_ND_INLINE Cycles kernel_conv_delayus_to_cycles( uint64_t microseconds )
{
	return microseconds * 1000; // TODO: better
}
MOSS_ND_INLINE Cycles kernel_conv_delayms_to_cycles( uint64_t milliseconds )
{
	return kernel_conv_delayus_to_cycles( 1000 * milliseconds );
}

MOSS_INLINE void kernel_acquire(void) { mutex_spin_acquire(&g_kernel.mutex); }
MOSS_INLINE void kernel_release(void) { mutex_spin_release(&g_kernel.mutex); }



/*typedef enum
{
	PANIC_TESTING,
	PANIC_ASSERT_FAILED
}
PANIC;

typedef struct
{
	PANIC panic;
	char const* msg;
}
PanicInfo;*/

__attribute__(( format(printf,1,0), noreturn ))
void kernel_panic_va( char const* fmt_cstr, va_list args );

// This is called by the C library when in freestanding configuration
__attribute__(( format(printf,1,2), noreturn ))
void __moss_kpanic   ( char const* fmt_cstr, ...          );
__attribute__(( format(printf,1,0), noreturn ))
void __moss_kpanic_va( char const* fmt_cstr, va_list args );

#define kernel_panic( FMT_CSTR, ... )\
	__moss_kpanic( FMT_CSTR __VA_OPT__(,) __VA_ARGS__ )



__attribute__(( always_inline, noreturn ))
inline void core_halt(void) { while (1) asm("hlt"); }



void kernel_main(void);



__attribute__(( noreturn ))
void kernel_demo_addr_map(void);

__attribute__(( noreturn ))
void kernel_demo_bootboot_screen(void);

__attribute__(( noreturn ))
void kernel_demo_mandelbrot( uint32_t core_ind );

__attribute__(( always_inline, noreturn ))
inline void kernel_demo_panic() { kernel_panic( "Oh no\n" ); }

__attribute__(( noreturn ))
void kernel_demo_stack( uint32_t core_ind );

/*__attribute__(( noreturn ))
void kernel_demo_proc_info()
{
	vec2i draw_pos = { 50, 200 };
	FILE file_fn = __moss_file_fn( &screen_write_ch, &draw_pos );
	__moss_fput_stack( &file_fn );

	fprintf( &file_fn, "\nAPIC ID: %u (%#x)\n", apic_id,apic_id );
}*/
