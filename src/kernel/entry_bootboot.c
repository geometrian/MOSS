//#include "stdafx.h"

#include "kernel.h"

#include "x86/cpuid.h"
#include "x86/descr_table.h"
#include "x86/misc.h"
#include "x86/ps2.h"

// Must come after "stdint.h"
#include <bootboot.h>



/* imported virtual addresses, see linker script */
extern BOOTBOOT bootboot;               // see bootboot.h
extern unsigned char environment[4096]; // configuration, UTF-8 text key=value pairs
extern uint8_t fb;                      // linear framebuffer mapped



#include <stdio.h>

int __moss_putc_stdout( int ch )
{
	return ch;
}
int __moss_putc_stderr( int ch )
{
	return ch;
}

int __moss_getc_stdin( void )
{
	return EOF;
}
int __moss_getch( void ) { return EOF; }



MOSS_ND_INLINE static struct AddrMap* _bootboot_mmap_parse(void)
{
	struct AddrMap* addr_map;

	// Count how many blocks there are
	size_t blocks_count = 0;
	{
		MMapEnt const* bootboot_block = &bootboot.mmap;
		while ( (uint8_t const*)bootboot_block < (uint8_t const*)&bootboot+bootboot.size )
		{
			++blocks_count;
			++bootboot_block;
		}
	}

	// Size of the `AddrMap` structure: the size itself plus the size of a block record times the
	// number of records (less the dummy 1 already stored in `AddrMap`).
	uint64_t map_sz = sizeof(struct AddrMap) + (blocks_count-1)*sizeof(struct AddrRange);

	// Place memory map in the first free block with a large enough page-aligned region.
	uint64_t largest_alignedfree_if_not_found = 0;
	for ( size_t k=0; k<blocks_count; ++k )
	{
		MMapEnt const* bootboot_block = &bootboot.mmap + k;
		if ( !MMapEnt_IsFree(bootboot_block) ) continue;

		uint64_t block_addr = MMapEnt_Ptr ( bootboot_block );
		uint64_t block_sz   = MMapEnt_Size( bootboot_block );

		uint64_t offset = ( 4096 - block_addr%4096 )%4096;
		if ( offset >= block_sz ) continue;
		block_sz -= offset;
		block_sz = ( block_sz / 4096 ) * 4096;
		largest_alignedfree_if_not_found = MAX( block_sz, largest_alignedfree_if_not_found );

		if ( block_sz < map_sz ) continue;
		addr_map = (struct AddrMap*)bootboot_block->ptr;
		goto FOUND_OKAY_MMAP_LOC;
	}
	kernel_panic(
		"Unable to find suitable memory block to store address map!\n"
		"Memory map has %zu blocks.\n"
		"%10lu [B] - required size of address map\n"
		"%10lu [B] - largest page-aligned subregion of any free block\n",
		blocks_count, map_sz, largest_alignedfree_if_not_found
	);
	FOUND_OKAY_MMAP_LOC:

	// Fill memory map
	addr_map->mem_size = 0;
	addr_map->blocks_count = blocks_count;
	for ( size_t k=0; k<blocks_count; ++k )
	{
		MMapEnt const* bootboot_block = &bootboot.mmap + k;

		switch (MMapEnt_Type( bootboot_block ))
		{
			default:
				// Doesn't seem to occur, but "Any other value is considered to be `MMAP_USED`."
				[[fallthrough]];
			case MMAP_USED:
				addr_map->blocks[k].type = ADDR_OTHER;
				break;
			case MMAP_FREE:
				addr_map->blocks[k].type = ADDR_MOSSPOOL;
				break;
			case MMAP_ACPI:
				addr_map->blocks[k].type = ADDR_ACPI;
				break;
			case MMAP_MMIO:
				addr_map->blocks[k].type = ADDR_MMIO;
				break;
		}

		addr_map->blocks[k].start = (AddrPhys)MMapEnt_Ptr ( bootboot_block );
		addr_map->blocks[k].size  =           MMapEnt_Size( bootboot_block );

		if ( addr_map->blocks[k].type != ADDR_MMIO )
		{
			addr_map->mem_size = (uint64_t)MAX(
				(AddrPhys)addr_map->mem_size,
				addr_map->blocks[k].start + addr_map->blocks[k].size
			);
		}
	}

	return addr_map;
}

static void _bootboot_percore_kmain(void)
{
	CPUID1 cpuid1 = x86_cpuid_1();
	uint32_t core_ind = cpuid1.hyperthread_ind;

	// BOOTBOOT enables SSE
	//_enable_vec_instructions();

	// BOOTBOOT gives us a valid GDT but neglects to tell us what's in it.  That's a problem since
	// e.g. we need to give valid segment selectors in IDT gates, and we can't do that without
	// knowing what segment descriptors are even available.
	x86_core_longmode64_setup_gdt64();

	if ( core_ind == 0 )
	{
		kernel_init();
		kernel_init_load_text();
		if ( bootboot.fb_scanline )
		{
			kernel_init_fbogrfx(
				COLOR_sBGR_A,
				(vec2u){ bootboot.fb_width, bootboot.fb_height },
				bootboot.fb_scanline,
				(void*)&fb
			);
		}

		//kernel_init_mmap( _bootboot_mmap_parse() );

		g_kernel.is_inited = true;
	}
	else
	{
		while ( !g_kernel.is_inited ) {}
	}

	(void)x86_ps2_init();
	(void)x86_ps2_redetect_devices();
	(void)x86_ps2_set_scanning(0,true);
	(void)x86_ps2_set_scanning(1,true);

	{
		x86_core_setup_idt64();

		x86_picpair_reinit( 32, 40 );
		x86_picpair_unmask_all();
		x86_picpair_mask(0); //No timer for now

		x86_core_interrupts_unmask();
		//kernel_test_div_zero();
	}

	(void)x86_ps2_set_scanning( 1, true );

	//kernel_demo_addr_map();
	//kernel_demo_bootboot_screen();
	//kernel_demo_mandelbrot( core_ind );
	//kernel_demo_panic();
	//kernel_demo_stack( core_ind );
	kernel_main();


	#if 0
	typedef struct
	{
		uintptr_t addr;
		uint64_t  size;
		uint8_t   type;
	}
	MemBlock;
	MemBlock blocks[64];
	size_t   blocks_count = 0;



	mmap_ent = &bootboot.mmap;
	while( (uint8_t const*)mmap_ent < (uint8_t const*)&bootboot+bootboot.size )
	{
		blocks[ blocks_count++ ] =
		(MemBlock){
			.addr = MMapEnt_Ptr( mmap_ent ),
			.size = MMapEnt_Size( mmap_ent ),
			.type = (uint8_t)MMapEnt_Type( mmap_ent )
		};
		// 0=reserved/unknown, 1=free, 2=acpi, 3=mmio

		++mmap_ent;
	}

	draw_line_v( &g_kernel.fbo, sRGB_A_(255,255,255,255),                50, 0,g_kernel.fbo.h );
	draw_line_v( &g_kernel.fbo, sRGB_A_(255,255,255,255), g_kernel.fbo.w-50, 0,g_kernel.fbo.h );

	sRGB_A const colors[4] =
	{
		sRGB_A_( 128,  0,128, 255 ), // reserved
		sRGB_A_( 192,192,192, 255 ), // free
		sRGB_A_( 255,128,  0, 255 ), // ACPI
		sRGB_A_(  64,128,255, 255 )  // MMIO
	};
	uintptr_t mem_size = blocks[blocks_count-1].addr + blocks[blocks_count-1].size;
	for ( size_t k=0; k<blocks_count; ++k )
	{
		float f_lo =  (float)blocks[k].addr                 / (float)mem_size;
		float f_hi = ((float)blocks[k].addr+blocks[k].size) / (float)mem_size;
		uint32_t x_lo = (uint32_t)roundf( f_lo * (float)(g_kernel.fbo.w-50-50) ) + 50;
		uint32_t x_hi = (uint32_t)roundf( f_hi * (float)(g_kernel.fbo.w-50-50) ) + 50;
		int y = 10 + (10+2)*k;
		if ( blocks[k].type > 3 ) blocks[k].type=0;
		draw_rect( &g_kernel.fbo, colors[blocks[k].type], Rect_( x_lo,y, x_hi-x_lo,10 ), 0 );

		draw_render_printf( &g_kernel.fbo,
			sRGB_A_( 255,255,255, 255 ), &g_kernel.font, vec2i_(2,y), "%2zu:", k
		);

		
	}
	#endif

	/*draw_render_printf( &g_kernel.fbo,
		sRGB_A_( 255,255,255, 255 ), &g_kernel.font, vec2i_( 10, 10+(y++)*16 ),
		"[ %#p, %#p ) (%10llu B), type %d", addr,addr+size, size, (int)type
	);*/

	/*draw_rect( &g_kernel.fbo, sRGB_A_(255,255,255,255), Rect_( 110,20, 20,10 ), 1 );
	draw_rect( &g_kernel.fbo, sRGB_A_(255,255,255,255), Rect_( 140,20, 20,10 ), 2 );
	draw_rect( &g_kernel.fbo, sRGB_A_(255,255,255,255), Rect_( 170,20, 20,10 ), 3 );

	switch ( core_ind )
	{
		case 0 : draw_rect( &g_kernel.fbo, sRGB_A_(  0,255,  0,255), Rect_(  20,50, 20,20 ), 1 ); break;
		case 1 : draw_rect( &g_kernel.fbo, sRGB_A_(  0,255,  0,255), Rect_(  45,50, 20,20 ), 1 ); break;
		case 2 : draw_rect( &g_kernel.fbo, sRGB_A_(  0,255,  0,255), Rect_(  70,50, 20,20 ), 1 ); break;
		case 3 : draw_rect( &g_kernel.fbo, sRGB_A_(  0,255,  0,255), Rect_(  95,50, 20,20 ), 1 ); break;
		default: draw_rect( &g_kernel.fbo, sRGB_A_(255,  0,255,255), Rect_( 115,50, 20,20 ), 1 ); break;
	}*/

	/*draw_at( &g_kernel.fbo, sRGB_A_(45,255,70,130), vec2i_( 31, 77) );
	draw_at( &g_kernel.fbo, sRGB_A_(45,255,70,130), vec2i_(126, 80) );
	draw_at( &g_kernel.fbo, sRGB_A_(45,255,70,130), vec2i_(700,151) );
	draw_at( &g_kernel.fbo, sRGB_A_(45,255,70,130), vec2i_(314,159) );*/

	//kernel_panic( "Oh no\n" );
	//__moss_assert_fail2( __FILE__,__LINE__, __PRETTY_FUNCTION__, "test %d",6 );
	//__moss_assert_impl( kernel.fbo.w == 33 );
}

/******************************************
* Entry point, called by BOOTBOOT Loader *
******************************************/
void _start(void)
{
	/*** NOTE: this code runs on all cores in parallel ***/

	/*
	BOOTBOOT seems to set up the stack like so (see also "doc/" for general information on how the
	stack is laid out):

		                                                             (NULL)
		(-16KiB * core index)                                          ^
		                                                               | (core=0: garbage=0x1d9416f8)
		0x0000'0000'0000'0008            call  ┌─> ┏━━━━━━━━━━━━━━━━━┓ | (core≠0: NULL)
		                                 info ─┤   ┃ Ret RIP address ╂ ┘   ^
		0x0000'0000'0000'0000 ━━━━━━━━━━━━━━━━━━━>━╋━━━━━━━━━━━━━━━━━┫     |
		                          `_start`     │   ┃ Ret RBP value   ╂ ─ ─ ┘
		                         STACK FRAME   └─>┏┻━━━━━━━━━━━━━━━━━┛ <━━━ (Current RBP)
		0xffff'ffff'ffff'fff8 ━━━━━━━━━━━━━━━━━━━━┛

		0xffff'ffff'ffff'fff0                                          <━━━ (Current RSP)

	This is not very good.  In particular, to detect the top of the stack on core 0, we rely on
	being able to read data stored at null.  Also RBP is garbage.

	The simplest thing to do would be to just forget that the return address exists and set the
	return RBP for core 0 to 0, which can be detected.  However, it's nice to be able to get both
	the return pointers at once, so the return RIP should be at a non-null address too.

	Thus, we do the following:

		                                                              (NULL)
		(-16KiB * core index)                                          ^
		                                                               | (NULL)
		0x0000'0000'0000'0000            call  ┌─> ┏━━━━━━━━━━━━━━━━━┓ |   ^
		                                 info ─┤   ┃ Ret RIP address ╂ ┘   |
		0xffff'ffff'ffff'fff8 ━━━━━━━━━━━━━━━━━━━>━╋━━━━━━━━━━━━━━━━━┫     |
		                          `_start`     │   ┃ Ret RBP value   ╂ ─ ─ ┘
		                         STACK FRAME   └─>┏┻━━━━━━━━━━━━━━━━━┛ <━┳━ (Current RBP)
		0xffff'ffff'ffff'fff0 ━━━━━━━━━━━━━━━━━━━━┛                      ┃
		                                                                 ┗━ (Current RSP)
	*/
	asm(
		"xor   rax, rax\n"

		"sub   rbp, 8\n"
		// (RSP needs to be set explicitly; too fragile to rely on it being in the right place)
		"mov   rsp, rbp\n"

		//"movabs   rax, 0x0123456789ABCDEF\n"
		"mov   [rbp  ], rax\n"
		//"movabs   rax, 0x0011223344556677\n"
		"mov   [rbp+8], rax\n"

		: : : "rax"
	);

	_bootboot_percore_kmain();

	// if we ever get here, hang
	core_halt();
}
