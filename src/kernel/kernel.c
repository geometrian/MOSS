#include "kernel.h"

#include "x86/ps2.h"



struct MOSS_Kernel g_kernel = { .is_inited=false };



void kernel_init(void)
{
	memset( &g_kernel, 0x00, sizeof(struct MOSS_Kernel) );

	g_kernel.mutex = (atomic_flag)ATOMIC_FLAG_INIT;

	inpdevs_init();
}

extern uint8_t _binary_data_fonts_Tamsyn8x16b_psf_start;
//extern uint8_t _binary_data_fonts_font5_5x9_psf_start;
void kernel_init_load_text(void)
{
	draw_load_font_ptr( &_binary_data_fonts_Tamsyn8x16b_psf_start, &g_kernel._font );
	//draw_load_font_ptr( &_binary_data_fonts_font5_5x9_psf_start, &g_kernel._font );
	g_kernel.grfx.font = &g_kernel._font;

	textui_init( &g_kernel._textui );
}

void kernel_init_fbogrfx( COLOR color_type, vec2u res, uint32_t scanline_sz, void* data )
{
	draw_img_init( &g_kernel._img_screen, color_type, res, data );

	draw_grfx_init( &g_kernel.grfx, &g_kernel._img_screen, &g_kernel._font );
}



int klogc( char ch )
{
	MOSS_DEBUG_ONLY( x86_serial_logc(ch); )
	return textui_putc( &g_kernel._textui, ch );
}
int klogs( char const* str )
{
	MOSS_DEBUG_ONLY( x86_serial_logs(str); )
	return textui_puts( &g_kernel._textui, str );
}
__attribute__(( format(printf,1,2) ))
int klogf ( char const* fmt_cstr,...          )
{
	va_list args;
	va_start( args, fmt_cstr );
	int ret = kvlogf( fmt_cstr, args );
	va_end( args );

	return ret;
}
__attribute__(( format(printf,1,0) ))
int kvlogf( char const* fmt_cstr,va_list args )
{
	MOSS_DEBUG_ONLY( x86_serial_vlogf(fmt_cstr,args); )
	return textui_vprintf( &g_kernel._textui, fmt_cstr,args );
}



__attribute__(( format(printf,1,0), noreturn ))
void kernel_panic_va( char const* fmt_cstr, va_list args )
{
	#ifdef MOSS_DEBUG
	x86_serial_logs ( "==== Kernel panic! ====\n" );
	x86_serial_vlogf( fmt_cstr, args );
	#endif

	g_kernel.grfx.text_pos = vec2i_( 50, 50 );
	g_kernel.grfx.text_x_wrap = 50;

	++g_kernel.panic.count;
	if      ( g_kernel.panic.count == 1 )
	{
		draw_fill_color( &g_kernel.grfx, sRGB_A_( 213,55,99, 255 ) );

		draw_set_color( &g_kernel.grfx, sRGB_A_( 255,255,255, 255 ) );

		draw_render_str( &g_kernel.grfx,
			"The MOSS kernel has detected a fatal error and has now terminated.\n"
			"\n"
			"This probably indicates a bug in a system-level program (such as a driver or the kernel itself),\n"
			"data corruption, or an uncorrectable hardware failure.  Additional information follows, which may\n"
			"help an expert diagnose the problem, but the computer has completely crashed, and there's nothing\n"
			"for it now except to shut it off.\n"
			"\n"
		);

		draw_render_vprintf( &g_kernel.grfx, fmt_cstr,args );
		draw_render_str( &g_kernel.grfx, "\n\n" );

		draw_render_stacktrace( &g_kernel.grfx );
		draw_render_ch( &g_kernel.grfx, '\n' );
	}
	else if ( g_kernel.panic.count == 2 )
	{
		draw_render_str( &g_kernel.grfx,
			"While attempting to output information, a second fatal error occurred.\n"
		);
	}
	// else something is seriously awry (e.g. with drawing strings), so just stop

	#if 0
	draw_fill_color( &g_kernel.grfx, sRGB_A_( 213,55,99, 255 ) );
	textui_draw( &g_kernel._textui, &g_kernel.grfx );
	#endif

	core_halt();
}

__attribute__(( format(printf,1,2), noreturn ))
void __moss_kpanic   ( char const* fmt_cstr, ...          )
{
	va_list args;
	va_start( args, fmt_cstr );
	kernel_panic_va( fmt_cstr, args );
	va_end( args );
}
__attribute__(( format(printf,1,0), noreturn ))
void __moss_kpanic_va( char const* fmt_cstr, va_list args )
{
	kernel_panic_va( fmt_cstr, args );
}



void kernel_main(void)
{
	struct Graphics* grfx = &g_kernel.grfx;

	struct Image backbuf;
	uint8_t* backbuf_data = grfx->fbo->data + 1080*1920*4;
	draw_img_init( &backbuf, grfx->fbo->color_type, vec2u_(1920,1080), backbuf_data );

	uint32_t frame = 0;
	while (1)
	{
		x86_ps2_handle_data();

		grfx->fbo = &backbuf;
		textui_draw( &g_kernel._textui, grfx );

		sRGB_A colors[3] = { sRGB_A_(255,0,0,255), sRGB_A_(0,255,0,255), sRGB_A_(0,0,255,255) };
		draw_rect_color( grfx, colors[frame%3], Rect_(1024-5-20,5,20,20), 0 );

		struct InpDev_Mouse const* mouse = inpdev_mouse_get(1);
		int mx = mouse->pos.x;
		int my = mouse->pos.y;
		draw_rect_color( grfx, sRGB_A_( 213,55,99, 255 ), Rect_(mx,my,5,5), 0 );

		grfx->fbo = &g_kernel._img_screen;
		draw_blit( grfx, &backbuf, vec2i_(0,0) );

		++frame;
	}
}



__attribute__(( noreturn ))
void kernel_demo_addr_map(void)
{
	char buf[ 10 ];

	g_kernel.grfx.text_pos = vec2i_( 5, 5 );
	g_kernel.grfx.text_x_wrap = 5;

	draw_render_printf( &g_kernel.grfx,
		"Found %llu address ranges:\n", g_kernel.addr_map->blocks_count
	);

	for ( size_t k=0; k<g_kernel.addr_map->blocks_count; ++k )
	{
		struct AddrRange const* blk = g_kernel.addr_map->blocks + k;

		draw_render_printf( &g_kernel.grfx,
			"  %2zu:   %#p -- %#p   (", k, blk->start,blk->start+blk->size-1
		);

		#if 1
		__moss_sprint_sz_4sigfigs( blk->size, buf, false );
		draw_render_printf( &g_kernel.grfx, "%s),   ", buf );
		#else
		draw_render_printf( &g_kernel.grfx, "%#11" PRIu64 " B),   ", blk->size );
		#endif

		char const* type_name;
		switch ( blk->type )
		{
			case ADDR_MOSSPOOL: type_name="available"         ; break;
			case ADDR_OTHER   : type_name="reserved / unknown"; break;
			case ADDR_ACPI    : type_name="ACPI"              ; break;
			case ADDR_MMIO    : type_name="MMIO"              ; break;
			__moss_switch_covered_enum;
		}

		draw_render_str( &g_kernel.grfx, type_name );

		draw_render_ch( &g_kernel.grfx, '\n' );
	}

	/*__moss_sprint_sz_4sigfigs( g_kernel.addr_map->mem_size, buf, false );
	draw_render_printf( &g_kernel.grfx, "Actual memory size seems to be %s.", buf );*/

	uint64_t total_size_all      = 0;
	uint64_t total_size_avail    = 0;
	uint64_t total_size_reserved = 0;
	uint64_t total_size_acpi     = 0;
	uint64_t total_size_mmio     = 0;
	for ( size_t k=0; k<g_kernel.addr_map->blocks_count; ++k )
	{
		struct AddrRange const* blk = g_kernel.addr_map->blocks + k;
		total_size_all += blk->size;
		switch ( blk->type )
		{
			case ADDR_MOSSPOOL: total_size_avail   +=blk->size; break;
			case ADDR_OTHER   : total_size_reserved+=blk->size; break;
			case ADDR_ACPI    : total_size_acpi    +=blk->size; break;
			case ADDR_MMIO    : total_size_mmio    +=blk->size; break;
			__moss_switch_covered_enum;
		}
	}
	uint64_t total_size_notavail = total_size_reserved + total_size_acpi + total_size_mmio;
	draw_render_printf( &g_kernel.grfx,
		"(total) %llu = (avail) %llu + (unavail) %llu\n"
		"(reserved) %llu\n(ACPI)     %llu\n(MMIO)     %llu\n",
		total_size_all, total_size_avail, total_size_notavail,
		total_size_reserved, total_size_acpi, total_size_mmio
	);

	core_halt();
}

__attribute__(( noreturn ))
void kernel_demo_bootboot_screen(void)
{
	// https://gitlab.com/bztsrc/bootboot/-/blob/master/mykernel/c/kernel.c#L56

	// cross-hair to see screen dimension detected correctly
	uint32_t w=g_kernel.grfx.fbo->w, h=g_kernel.grfx.fbo->h;
	draw_line_h_color( &g_kernel.grfx, sRGB_A_( 255,255,255, 255 ), 0,w, h/2 );
	draw_line_v_color( &g_kernel.grfx, sRGB_A_( 255,255,255, 255 ), w/2, 0,h );

	// red, green, blue boxes in order
	draw_rect_color( &g_kernel.grfx, sRGB_A_(255,0,0,255), Rect_( 20,20, 20,20 ), 0 );
	draw_rect_color( &g_kernel.grfx, sRGB_A_(0,255,0,255), Rect_( 50,20, 20,20 ), 0 );
	draw_rect_color( &g_kernel.grfx, sRGB_A_(0,0,255,255), Rect_( 80,20, 20,20 ), 0 );

	// say hello
	draw_set_color( &g_kernel.grfx, sRGB_A_( 255,255,255, 255 ) );
	draw_render_str( &g_kernel.grfx, "Hello from a simple BOOTBOOT kernel" );

	core_halt();
}

static atomic_uint _mandelbrot_scanline = 0u;
MOSS_ND_INLINE static sRGB_A _mandelbrot_color(
	float fx,float fy, float cen_r,float cen_i, float aspect, float zoom
) {
	sRGB_A const colors[ 16 + 1 ] =
	{
		sRGB_A_( 241, 233, 191, 255 ),
		sRGB_A_( 248, 201,  95, 255 ),
		sRGB_A_( 255, 170,   0, 255 ),
		sRGB_A_( 204, 108,   0, 255 ),
		sRGB_A_( 153,  87,   0, 255 ),
		sRGB_A_( 106,  52,   3, 255 ),
		sRGB_A_(  66,  30,  15, 255 ),
		sRGB_A_(  25,   7,  26, 255 ),
		sRGB_A_(   9,   1,  47, 255 ),
		sRGB_A_(   4,   4,  73, 255 ),
		sRGB_A_(   0,   7, 100, 255 ),
		sRGB_A_(  12,  44, 138, 255 ),
		sRGB_A_(  24,  82, 177, 255 ),
		sRGB_A_(  57, 125, 209, 255 ),
		sRGB_A_( 134, 181, 229, 255 ),
		sRGB_A_( 211, 236, 248, 255 ),
		sRGB_A_(   0,   0,   0, 255 )
	};

	vec2f2x c, z;
	//c.x = cen_r + lerp( -aspect,aspect, fx )*zoom;
	//c.y = cen_i + lerp( -1.0f  ,1.0f  , fy )*zoom;
	c.x = cen_r + aspect*zoom*( 2.0f*fx - 1.0f );
	c.y = cen_i +        zoom*( 2.0f*fy - 1.0f );

	z = c;
	int k;
	int const KMAX = 50;
	for ( k=0; k<KMAX; ++k )
	{
		z = (vec2f2x){{ z.a*z.a-z.b*z.b+c.a, 2.0f*z.a*z.b+c.b }};
		if ( z.a*z.a + z.b*z.b > 2.0f*2.0f ) break;
	}

	if ( k == KMAX ) k= 16;
	else             k%=16;
	return colors[k];
}
__attribute__(( noreturn ))
void kernel_demo_mandelbrot( uint32_t core_ind )
{
	struct Image* img = g_kernel.grfx.fbo;

	float aspect = (float)img->w / (float)img->h;
	float zoom = 1.3f;
	float cen_r = -0.5f;
	float cen_i =  0.0f;

	while ( true )
	{
		uint32_t y = atomic_fetch_add( &_mandelbrot_scanline, 1 );
		if ( y >= img->h ) break;

		for ( uint32_t x=0; x<img->w; ++x )
		{
			sRGB_A samples[16];
			for ( uint32_t suby=0; suby<4; ++suby )
			for ( uint32_t subx=0; subx<4; ++subx )
			{
				float subxf = ((float)subx+0.5f)*(1.0f/4.0f);
				float subyf = ((float)suby+0.5f)*(1.0f/4.0f);
				float fx =        ((float)x+subxf)/(float)img->w;
				float fy = 1.0f - ((float)y+subyf)/(float)img->h;
				samples[ suby*4 + subx ] = _mandelbrot_color( fx,fy, cen_r,cen_i, aspect,zoom );
			}
			sRGB_A color = color_rgba_avg( samples, 16 );

			draw_at_color( &g_kernel.grfx, color, vec2i_(x,y) );
		}
	}

	if ( core_ind == 0 )
	{
		draw_set_color( &g_kernel.grfx, sRGB_A_( 0,0,0, 255 ) );
		g_kernel.grfx.text_pos = vec2i_( img->w/2-8*g_kernel._font.glyph_w, 5 );
		draw_render_str( &g_kernel.grfx, "Render complete!" );
	}

	core_halt();
}

static int _stack_putc( int ch, void* data )
{
	return klogc( ch );
}
__attribute__(( noreturn ))
void kernel_demo_stack( uint32_t core_ind )
{
	FILE file_fn = __moss_file_fn( &_stack_putc, NULL );
	__moss_fput_stack( &file_fn );

	core_halt();
}
