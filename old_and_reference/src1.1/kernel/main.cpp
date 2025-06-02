#if 0
#include "cpu.hpp"
#include "gdt.hpp"



namespace MOSS
{



/*
TextUI
Note that UEFI doesn't do text mode; it fakes it by using a graphics mode.
*/
class TextUI final : public MOSS::Term::BasicTextUI
{
	public:
		TextUI() noexcept : MOSS::Term::BasicTextUI( 60,40, 20 ) {}

		void debug_print_str( char const* str ) const noexcept override
		{
			__moss_uefi_basic_print_str( str );
		}

		void bell() noexcept override
		{
			CPU::speaker_emit_bell( []( uint32 microseconds )
			{
				g_uefi_sys_table->BootServices->Stall( microseconds );
			} );
		}
		void newline() noexcept override
		{
			//refresh();
		}

		void refresh_set_draw_pos( uint32 col_ind, uint32 row_ind ) noexcept override
		{
			g_uefi_sys_table->ConOut->SetCursorPosition( g_uefi_sys_table->ConOut, col_ind,row_ind );
		}
		void refresh_set_draw_attribs( CharAttribs const& attribs ) noexcept override
		{
			uint8 color = srgb_to_uefi_bitfield( attribs.color_fg, attribs.color_bg );
			g_uefi_sys_table->ConOut->SetAttribute( g_uefi_sys_table->ConOut, color );
		}
		void refresh_draw_row_range( Char const* rng_start, Char const* rng_end ) noexcept override
		{
			CHAR16 buf[ 512 ];
			CHAR16* dst = buf;
			for ( Char const* src=rng_start; src!=rng_end; ++src )
			{
				char ch = (char)(src->ch);
				if ( ch == '\0' ) ch='.';
				*(dst++) = ch;
			}
			*dst = '\0';

			g_uefi_sys_table->ConOut->OutputString( g_uefi_sys_table->ConOut, buf );
		}
};

static TextUI* g_text_ui;



extern "C" int __moss_putc_stdout( int ch )
{
	MOSS::g_text_ui->write( ch );
	MOSS::g_text_ui->refresh();
	//__moss_uefi_basic_print_ch( ch );
	return ch;
}
extern "C" int __moss_putc_stderr( int ch )
{
	MOSS::g_text_ui->write( ch );
	MOSS::g_text_ui->refresh();
	//__moss_uefi_basic_print_ch( ch );
	return ch;
}

extern "C" int __moss_getc_stdin()
{
	EFI_INPUT_KEY key;
	MOSS::g_uefi_sys_table->ConIn->ReadKeyStroke( MOSS::g_uefi_sys_table->ConIn, &key );
	return (char)key.UnicodeChar;
}
extern "C" int __moss_getch()
{
	return '?';
}



static void text_test()
{
	__moss_uefi_basic_print_str("Text test:\r\n");

	for     ( uint32 bg=0; bg<=  0b111; ++bg ) // 3 bits for background
	{
		for ( uint32 fg=0; fg<=0b1'111; ++fg ) // 4 bits for foreground
		{
			g_uefi_sys_table->ConOut->SetAttribute( g_uefi_sys_table->ConOut, (bg<<4)|fg );
			__moss_uefi_basic_print_ch( '#' );
		}
		__moss_uefi_basic_print_str( "\r\n" );
	}

	g_uefi_sys_table->ConOut->SetAttribute( g_uefi_sys_table->ConOut, 0b000'0'111 );
	__moss_uefi_basic_print_str("Done!\r\n");
}



extern "C" int main( int, char*[] )
{
	//Setup text terminal so we can see what we're doing
	TextUI text_ui;
	g_text_ui = &text_ui;

	//text_test(); halt();

	for ( unsigned i=0; i<16; ++i )
	{
		auto const& col = MOSS::Color::vga_colors[i];
		std::printf( "%2u => %u,%u,%u\n", i, col.r,col.g,col.b );
	}
	halt();

	//Say hello
	//	https://en.wikipedia.org/wiki/ANSI_escape_code#Colors
	std::fputs(
		"\e[32m" //Green
		"         _    _ "  "  ____  "   "  ____ "   "  ____ "   "\n"
		"       /| \\  / |" " /  _ \\ "  " /___/ "   " /___/ "   "\n"
		"       ||  \\/  |" "|| / \\ |"  "|\\____ "  "|\\____ "  "\n"
		"       || |\\/| |" "|| \\_/ |"  " \\____\\" " \\____\\" "\n"
		"       ||_| ||_|"  "\\ \\___/ " " _____/"   " _____/"   "\n"
		"       /_/  /_/ "  " \\___/  "  "/____/ "   "/____/ "   "\n"
		"                "  "        "   "       "   "       "   "\n"
		"\e[95m" //Magenta
		"  The Minimal Operating System that Sucks\n"
		"\e[35m" //Purple
		"    by Agatha Mallett\n\n"
		"\e[0m" //Reset
	, stdout );
	#if 0
	CPU::speaker_emit_bell( []( uint32 microseconds )
	{
		g_uefi_sys_table->BootServices->Stall( microseconds );
	} );
	#endif
	std::printf( MOSS_MSG "MOSS UEFI kernel loader startup\n" );

	__moss_uefi_basic_print_str("basic 3\r\n");

	/*
	There is a watchdog timer that reboots after 5 minutes because it assumes your EFI application
	hung.  If the bootloader runs for 5 minutes (e.g. because it's interacting with the user), then
	this trips.  Disable it.  We have to disable it before returning anyway (if we do indeed
	return).  See also:
		https://wiki.osdev.org/UEFI#My_UEFI_application_hangs.2Fresets_after_about_5_minutes
		https://uefi.org/specs/UEFI/2.10/07_Services_Boot_Services.html#efi-boot-services-setwatchdogtimer
	*/
	g_uefi_sys_table->BootServices->SetWatchdogTimer( 0, 0, 0,nullptr );


	std::printf( MOSS_MSG "UEFI startup 2!\n" );

	std::printf( MOSS_MSG "Floating point: %f\n", std::exp(1.0) );

	//std::printf( MOSS_MSG "Firmware: \"%ls\"\n", sys_table->firmware_vendor );

	/*std::printf( MOSS_MSG "Load GDT:\n" );
	GDT::setup();

	*((int*)0xb8000)=0x07690748;
	std::fputs("Done?\n",stdout);*/

	halt();
}



}



#if 0
class StdIOOutputBuffer final
{
	public:
		static constexpr size_t SZ_LINE = 256;
		static_assert( SZ_LINE > 0 );

	private:
		EFI::CHAR16 _buf_line[ SZ_LINE ];
		size_t _count_line = 0;

	public:
		EFI::SIMPLE_TEXT_OUTPUT_PROTOCOL* sys;

	public:
		void putc( char ch ) noexcept
		{
			if ( ch == '\n' ) _buf_line[ _count_line++ ]='\r';
			_buf_line[ _count_line++ ] = ch;

			//Must always have space for at least three characters ("\r\n\0")
			if ( _count_line>SZ_LINE-3 || ch=='\n' ) flush();
		}
		void flush() noexcept
		{
			_buf_line[ _count_line ] = '\0';
			EFI::g_sys_table->text_out->print( _buf_line );
			_count_line = 0;
		}
};

StdIOOutputBuffer g_sysout;

extern "C" int __moss_putc_stdout( int ch )
{
	g_sysout.putc( ch );
	return ch;
}
extern "C" int __moss_putc_stderr( int ch )
{
	g_sysout.putc( ch );
	return ch;
}
#endif



/* These magic symbols are provided by the linker.  */
/*
extern "C" void (*__preinit_array_start []) (void);
extern "C" void (*__preinit_array_end []) (void);
extern "C" void (*__init_array_start []) (void);
extern "C" void (*__init_array_end []) (void);

extern "C" void _init (void);

extern "C" void __libc_init_array (void)
{
  size_t count;
  size_t i;

  count = __preinit_array_end - __preinit_array_start;
  for (i = 0; i < count; i++)
    __preinit_array_start[i] ();

  //_init ();

  count = __init_array_end - __init_array_start;
  for (i = 0; i < count; i++)
    __init_array_start[i] ();
}
*/



extern "C" void _start();

//UEFI Image Entry Point
//	See UEFI Standard §4.1. "UEFI Image Entry Point"
//	https://uefi.org/specs/UEFI/2.10/04_EFI_System_Table.html#uefi-image-entry-point
EFI_STATUS EFIAPI efi_main( IN EFI_HANDLE /*image_handle*/, IN EFI_SYSTEM_TABLE* sys_table )
{
	MOSS::g_uefi_sys_table = sys_table;
	_start();

	return 0;
}
#endif



#if 1


#include <Uefi.h>
#include <Library/UefiLib.h>

struct GlobalType {
	int var;
	GlobalType () { var=1; }
};
GlobalType foo, bar, baz;

EFI_STATUS EFIAPI efi_main( IN EFI_HANDLE /*image_handle*/, IN EFI_SYSTEM_TABLE* sys_table ) {
	sys_table->ConOut->OutputString( sys_table->ConOut, (CHAR16*)L"Var is: \"" );
	CHAR16 str[3+1] = { (CHAR16)(foo.var+'0'), (CHAR16)(bar.var+'0'), (CHAR16)(baz.var+'0'), '\0' };
	sys_table->ConOut->OutputString( sys_table->ConOut, str );
	sys_table->ConOut->OutputString( sys_table->ConOut, (CHAR16*)L"\"" );
	while (1) asm("hlt");
	return 0;
}

#endif
