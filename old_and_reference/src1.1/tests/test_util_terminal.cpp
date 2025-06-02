#include "../libmoss-stdcpp/cstdio"

#include "../libmoss-util/terminal.hpp"



/*
Text UI backed by hosted system, not quite a terminal
*/
class TextUI final : public MOSS::Term::BasicTextUI
{
	public:
		TextUI(  uint32 buffer_w,uint32 buffer_h, uint32 view_h ) noexcept :
			MOSS::Term::BasicTextUI( buffer_w,buffer_h, view_h )
		{
			refresh();
		}

		void bell() noexcept override { _emit( '\a' ); }

	private:
		void _emit( char ch ) noexcept { std::fputc( ch, stdout ); }

	public:
		void refresh_set_draw_pos( uint32 col_ind, uint32 row_ind ) noexcept override
		{
			static CharInd old = { 0, 0 };
			if ( old == CharInd{ col_ind, row_ind } ) return;
			std::printf( "\e[%d;%dH", row_ind+1, col_ind+1 );
			old = { col_ind, row_ind };
		}
		void refresh_set_draw_attribs( CharAttribs const& attribs ) noexcept override
		{
			#if 0
				auto const& fg = attribs.color_fg;
				auto const& bg = attribs.color_bg;
				std::printf(
					"\e[" "38;2;%d;%d;%d" ";" "48;2;%d;%d;%d" "m",
					fg.r,fg.g,fg.b, bg.r,bg.g,bg.b
				);
			#else
				uint8 color = srgb_to_uefi_bitfield( attribs.color_fg, attribs.color_bg );
				struct Color final
				{
					uint8 r, g, b;
					Color( uint8 val ) :
						r( (val&0x4) * ((val&0x80)?255:170) ),
						g( (val&0x2) * ((val&0x80)?255:170) ),
						b( (val&0x1) * ((val&0x80)?255:170) )
					{}
				};
				Color fg(color&0x0F), bg(color>>4);
				std::printf(
					"\e[" "38;2;%d;%d;%d" ";" "48;2;%d;%d;%d" "m",
					fg.r,fg.g,fg.b, bg.r,bg.g,bg.b
				);
			#endif
		}
		void refresh_draw_row_range( Char const* rng_start, Char const* rng_end ) noexcept override
		{
			do
			{
				char ch = (char)rng_start->ch;
				std::fputc( ch=='\0'?' ':ch, stdout );
			}
			while ( ++rng_start < rng_end );
		}

		void output_charcodes() noexcept
		{
			int ch = std::getch();
			writef( "[ 0x%.2x", ch );
			if ( std::isprint(ch) ) writef( " ('%c')", ch );
			write( " ]\n" );
		}
};



void test_text() noexcept
{
	TextUI ui( 60,40, 20 );
	//ui.cursor_move_relative( 0, 8 );

	//ui.write("this\r\nis a test\r\nof the\r\nemergency\r\nbroadcasting\r\nsystem");

	ui.write(
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
	);

	ui.refresh();

	(void)std::getchar();
}

void test_print_charcodes() noexcept
{
	TextUI ui( 30,20, 5 );

	while ( true )
	{
		ui.output_charcodes();
		ui.refresh();
	}
}

void test_terminal() noexcept
{
	TextUI ui( 30,20, 5 );

	while ( true )
	{
		bool okay = true;
		int ch1 = std::getch();
		if ( ch1 == 0xe0 )
		{
			int ch2 = std::getch();
			if      ( ch2 == 'K' ) okay=ui.cursor_move_relative( -1,  0 ); // left
			else if ( ch2 == 'M' ) okay=ui.cursor_move_relative(  1,  0 ); // right
			else if ( ch2 == 'P' ) okay=ui.cursor_move_relative(  0,  1 ); // down
			else if ( ch2 == 'H' ) okay=ui.cursor_move_relative(  0, -1 ); // up
			else okay=false;
		}
		else
		{
			okay &= ui.write( ch1 );
		}

		if ( !okay ) ui.bell();

		ui.refresh();
	}
}



int main( int /*arc*/, char* /*argv*/[] )
{
	test_text();
	//test_print_charcodes();
	//test_terminal();
	return 0;
}


#if 0
/*
Terminal backed by hosted system
*/
class TerminalHosted final : public MOSS::TerminalBase
{
	public:
		TerminalHosted() : MOSS::TerminalBase( 20, 30,5 )
		{
			_rerender_all_lines_and_cursor();
		}

	protected:
		

		void _rerender_all_lines_and_cursor() noexcept override
		{
			
			_rerender_cursor();
		}
		void _rerender_curr_line_and_cursor() noexcept override
		{
			//_set_pos( 0, _screen.cursor_ind[1] );
			//_rerender_cursor();
			_rerender_all_lines_and_cursor();
		}
		void _rerender_cursor              () noexcept override {
			_set_pos( _screen.cursor_ind[0]+3, _screen.cursor_ind[1] );
		}

	private:
		

	
		void handle_input() noexcept;
};

void TerminalHosted::handle_input() noexcept
{
	/*
	Windows seems to give an output of one or two bytes for each key or key-combination.  The
	following seems to cover everything except (most) CTRL-ALT-<?> codes.
	*/
	int ch1 = std::getch();
	switch ( ch1 )
	{
		/*case 0x00:
		{
			int ch2 = std::getch();
			switch ( ch2 )
			{
				case 0x03: break; // CTRL-2
				case 0x0e: break; // CTRL-ALT-<backspace>
				case 0x3b: break; // F1
				case 0x3c: break; // F2
				case 0x3d: break; // F3
				case 0x3e: break; // F4
				case 0x3f: break; // F5
				case 0x40: break; // F6
				case 0x41: break; // F7
				case 0x42: break; // F8
				case 0x43: break; // F9
				case 0x44: break; // F10
				case 0x5e: break; // CTRL-F1
				case 0x5f: break; // CTRL-F2
				case 0x60: break; // CTRL-F3
				case 0x61: break; // CTRL-F4
				case 0x62: break; // CTRL-F5
				case 0x63: break; // CTRL-F6
				case 0x64: break; // CTRL-F7
				case 0x65: break; // CTRL-F8
				case 0x66: break; // CTRL-F9
				case 0x67: break; // CTRL-F10
				case 0x9f; break; // CTRL-ALT-END
				default: break;
			}
			break;
		}*/
		case 0x01: cursor_home();          return; // CTRL-A, "home"
		case 0x02: cursor_backward();      return; // CTRL-B, "cursor backward"
		case 0x03: cmd_terminate();        return; // CTRL-C, "terminate command"
		case 0x04: edit_del_1_after();     return; // CTRL-D, "delete character at / after cursor"
		case 0x05: cursor_end ();          return; // CTRL-E, "end"
		case 0x06: cursor_forward();       return; // CTRL-F, "cursor forward"
		//case 0x07: break; // CTRL-G, "exist history search mode"
		case 0x08: edit_del_1_before();    return; // CTRL-H, "delete character before cursor"
		//case 0x09: break; // CTRL-I,
		//case 0x0a: break; // CTRL-J, ENTER
		case 0x0b: edit_del_all_after();   return; // CTRL-K, "delete from cursor to end"
		case 0x0c: screen_clear();         return; // CTRL-L, "clear screen"
		//case 0x0d: break; // CTRL-M,
		case 0x0e: break; // CTRL-N, "(replace line with) next command from history (see also CTRL-P)"
		//case 0x0f: break; // CTRL-O,
		//case 0x10: break; // CTRL-P, "(replace line with) previous command from history (see also CTRL-N)"
		//case 0x11: break; // CTRL-Q, "re-allow command's output to screen (see also CTRL-S)"
		//case 0x12: break; // CTRL-R, "enter history search mode"
		//case 0x13: break; // CTRL-S, "stop command's output to screen (see also CTRL-Q)"
		case 0x14: edit_transpose_at();    return; // CTRL-T, "transpose characters before and at / after cursor"
		case 0x15: edit_del_all_before();  return; // CTRL-U, "delete from beginning to cursor"
		//case 0x16: break; // CTRL-V,
		case 0x17: edit_del_word_before(); return; // CTRL-W, "delete from beginning of word to cursor"
		//case 0x18: break; // CTRL-X,
		//case 0x19: break; // CTRL-Y, "paste"
		//case 0x1a: break; // CTRL-Z, "suspend command"
		//case 0x1b: break; // ESC, CTRL-[ "escape"
		//case 0x1c: break; /*      CTRL-\ */
		//case 0x1d: break; //      CTRL-]
		//case 0x7f: break; // CTRL-<BS>
		case 0xe0:
		{
			int ch2 = std::getch();
			switch ( ch2 )
			{
				case 0x52: edit_toggle_insert_mode(); return; // INS, "toggle insert mode"
				case 0x53: edit_del_1_after();        return; // DEL, "character at / after cursor"
				case 0x47: cursor_home();             return; // HOME
				case 0x48: cursor_up  ();             return; // <up-arrow>
				case 0x49: cursor_pageup();           return; // PGUP
				case 0x4b: cursor_backward();         return; // <left-arrow>,  "cursor backward"
				case 0x4d: cursor_forward ();         return; // <right-arrow>, "cursor forward"
				case 0x4f: cursor_end ();             return; // END, "end"
				case 0x50: cursor_down();             return; // <down-arrow>
				case 0x51: cursor_pagedown();         return; // PGDN
				case 0x73: cursor_home();             return; // CTRL-LEFT, "home"
				case 0x74: cursor_end();              return; // CTRL-RIGHT, "end"
				//case 0x75: break; // CTRL-END
				//case 0x76: break; // CTRL-PGUP
				//case 0x77: break; // CTRL-HOME
				//case 0x85: break; // F11
				//case 0x86: break; // F12, CTRL-PGDN
				//case 0x8d: break; // CTRL-UP
				//case 0x89: break; // CTRL-F11
				//case 0x8a: break; // CTRL-F12
				//case 0x91: break; // CTRL-DOWN
				//case 0x92: break; // CTRL-INS
				//case 0x93: break; // CTRL-DEL
				default: break;
			}
			break;
		}
		default: break;
	}
	if ( !write(ch1) ) [[unlikely]] bell();
}



int main( int /*arc*/, char* /*argv*/[] )
{
	//std::printf( "[%*.0s]\n", 3,"" );
	//std::getchar();

	TerminalHosted term;
	while ( true )
	{
		#if 0
		int ch = std::getch();
		if ( ch == 0xe0 )
		{
			/*ch = std::getch();
			bool okay;
			if      ( ch == 'K' ) okay=term.move_cursor_relative( -1,  0 ); // left
			else if ( ch == 'M' ) okay=term.move_cursor_relative(  1,  0 ); // right
			else if ( ch == 'P' ) okay=term.move_cursor_relative(  0,  1 ); // down
			else if ( ch == 'H' ) okay=term.move_cursor_relative(  0, -1 ); // up
			else if ( ch == 'G' ) okay=term.move_cursor_absolute_x( 0 ); // home
			else if ( ch == 'O' ) okay=term.move_cursor_absolute_x( term.get_curr_line().length() ); // end
			//else if ( ch == 'S' ) // delete
			//else if ( ch == 'R' ) // insert
			else okay=false;
			if ( !okay )
			{
				term.handle_printf( "'%2x'", ch );
				term.bell();
			}
			continue;*/
			int ch2 = std::getch();
			term.handle_printf( "['0xe0','0x%.2x']", ch2 );
		}
		else
		{
			term.handle_printf( "['0x%.2x']", ch );
		}
		continue;

		term.handle_print( ch );
		#endif

		term.output_charcodes();
	}

	/*std::printf("[Unformatted]\n");
	std::printf("[Bell: \"\a\"]\n");
	std::printf("[\e[32mGreen\e[0m]\n");

	(void)std::getchar();*/

	return 0;
}
#endif
