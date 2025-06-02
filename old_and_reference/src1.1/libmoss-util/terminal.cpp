#include "terminal.hpp"

//#include "../libmoss-stdc/__env.h"



namespace MOSS::Term
{



//https://en.wikipedia.org/wiki/ANSI_escape_code

/*
CSI Sequence

1: "\e[" (control sequence introducer, 'CSI')

2: 0 or more parameter bytes in range:
   "0123456789:;<=>?" [0x30,0x3f]

3: 0 or more intermediate bytes in range:
   " !\"#$%&'()*+,-./" [0x20,0x2f]
   Note '"' is in there and was written escaped

4: 1 final byte in range:
   "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~" [0x40,0x7E]
*/



/*
Parses the semicolon-separated integer interior of an escape sequence (e.g. given "\e[2;1H", give it
"2;1") and returns a list (in this case, {2,1}).  Omitted elements are returned as -1, and there is
always construed to be at least one element.  If the contents contain something else, returns an
empty list.

Examples:
	Sequence    Passed Here   Result    Note
	"\e[2;1H"   "2;1"         {2,1}     (basic)
	"\e[2A"     "2"           {2}       (basic)
	"\e[;3H"    ";3"          {-1,3}    (omitted element(s) treated as -1)
	"\e[A"      ""            {-1}      (omitted element(s) treated as -1, minimum one element)
	"\e[?25h"   "?25"         {}        (invalid / unknown sequence)

TODO: `string_view`
*/
[[nodiscard]] inline
std::vector<int32> _parse_semi_int_seq( std::string const& esc_seq_contents ) noexcept
{
	std::vector< std::string > toks;
	std::string tok;
	std::vector<int32> ret;

	//Split into a bunch of digit-only-containing tokens
	char const* ptr = esc_seq_contents.c_str();
	while ( true )
	{
		if      ( *ptr == ';' )
		{
			toks.emplace_back( std::move(tok) );
			__moss_assert_impl( tok.empty() ); // After move
		}
		else if ( std::isdigit(*ptr) )
		{
			tok += *ptr;
		}
		else if ( *ptr == '\0' ) break;
		else return {};

		++ptr;
	}
	toks.emplace_back( tok );

	//Parse each as an integer
	for ( std::string const& tok : toks )
	{
		int num;
		if ( !tok.empty() )
		{
			num = std::atoi( tok.c_str() );
			__moss_assert_impl( num >= 0 );
		}
		else num=-1;
		ret.emplace_back( num );
	}

	return ret;
}



[[nodiscard]] CharMatrix::CharAttribs CharMatrix::CharAttribs::get_default()
{
	CharAttribs ret;
	std::memset( &ret, 0x00, sizeof(CharAttribs) );

	#if MOSS_ENV == MOSS_ENV_EMBEDWIN
		ret.color_fg = ret.color_lines = {  0,170, 0 };
		ret.color_bg =                   { 32, 32, 0 };
		//ret.color_fg = ret.color_lines = Color::vga_colors[ 7 ];
		//ret.color_bg =                   Color::vga_colors[ 3 ];
		/*std::printf(
			"$> "
			//"\e[42m"
			"\e[48;2;0;64;0m"
			"%s"
			//"\e[43m"
			"\e[48;2;32;32;0m"
			"%*.0s"
			"\e[0m\n",
			str.c_str(), pad,""
		);*/
	#else
		ret.color_fg = ret.color_lines = Color::vga_colors[ 7 ];
		ret.color_bg =                   Color::vga_colors[ 0 ];
	#endif

	return ret;
}

[[nodiscard]] bool CharMatrix::apply_sgr_esc_seq(
	CharAttribs const& attribs, std::vector<int32> const& cmds, CharAttribs* result
) noexcept {
	if ( cmds.empty() ) [[unlikely]] return false;

	auto parse_8bit_or_24bit_color =
	[&]( std::vector<int32>::const_iterator* iter_ptr, Color::sRGB* color ) -> bool
	{
		std::vector<int32>::const_iterator& iter = *iter_ptr;

		// Advance past the 38, 48, or 58 command this came from, and ensure we have a next command
		if ( ++iter == cmds.cend() ) return false;

		// Get color type and ensure we have a color definition
		int32 type = *iter;
		if ( ++iter == cmds.cend() ) return false;

		if      ( type == 5 ) //8-bit color
		{
			int32 color_cmd = *(iter++);
			if      ( color_cmd <=  15 ) *color=Color::vga_colors[ color_cmd ];
			else if ( color_cmd <= 231 )
			{
				color_cmd -= 16;
				uint8 b = color_cmd % 6;
				color_cmd /= 6;
				uint8 g = color_cmd % 6;
				color_cmd /= 6;
				uint8 r = color_cmd;
				__moss_assert_impl( r<6 && g<6 && b<6 );
				*color = { Color::srgb_gamma_6[r], Color::srgb_gamma_6[g], Color::srgb_gamma_6[b] };
			}
			else if ( color_cmd <= 255 )
			{
				uint8 val = Color::srgb_gamma_24[ color_cmd - 232 ];
				*color = { val, val, val };
			}
			else return false;
		}
		else if ( type == 2 ) //24-bit color
		{
			int32 r = *iter;
			if ( r<0 || r>255 || ++iter==cmds.cend() ) return false;
			int32 g = *iter;
			if ( g<0 || g>255 || ++iter==cmds.cend() ) return false;
			int32 b = *iter;
			if ( b<0 || b>255                        ) return false;
			*color = { (uint8)r, (uint8)g, (uint8)b };
		}
		else return false;
		return true;
	};

	*result = attribs;
	bool reverse=false, conceal=false;
	for ( auto iter=cmds.cbegin(); iter!=cmds.cend(); ++iter )
	{
		int32 cmd = *iter;
		switch ( cmd )
		{
			case 0: // reset / normal
			{
				uint8 old_is_dirty = result->is_dirty;
				*result = CharMatrix::CharAttribs::get_default();
				result->is_dirty = old_is_dirty;
				break;
			}
			case 1: // bold or increased intensity
				result->font_bold = 1;
				break;
			case 2: // bold or decreased intensity
				result->font_faint = 1;
				break;
			case 3: // italic
				result->font_italic = 1;
				break;
			case 4: // underline
				result->lines_under = 1;
				break;
			case 5: // slow blink
				result->blink = 1;
				break;
			case 6: // rapid blink
				result->blink = 2;
				break;
			case 7: // reverse foreground and background
				reverse = true;
				break;
			case 8: // conceal / hide
				conceal = true;
				break;
			case 9: // strikethrough
				result->line_through = 1;
				break;
			// select fonts
			case 10: case 11: case 12: case 13: case 14: [[fallthrough]];
			case 15: case 16: case 17: case 18: case 19: [[fallthrough]];
			case 20:
				result->font_index = cmd - 10;
				break;
			case 21: // double-underline (not-bold on some bad terminals)
				result->lines_under = 2;
				break;
			case 22: // normal intensity
				result->font_bold = result->font_faint = 0;
				break;
			case 23: // not italic or blackletter
				result->font_italic = 0;
				if ( result->font_index == 20 ) result->font_index=0;
				break;
			case 24: // no underline(s)
				result->lines_under = 0;
				break;
			case 25: // no blinking
				result->blink = 0;
				break;
			case 26: // proportional font
				result->font_proportional = 1;
				break;
			case 27: // not reversed
				reverse = false;
				break;
			case 28: // not concealed
				conceal = false;
				break;
			case 29: // not crossed out
				result->line_through = 0;
				break;
			case 30: case 31: case 32: case 33: [[fallthrough]]; // 3-bit foreground
			case 34: case 35: case 36: case 37:
				result->color_fg = Color::vga_colors[ cmd - 30 ];
				break;
			case 38: // 8-bit or 24-bit foreground
				if ( !parse_8bit_or_24bit_color( &iter, &result->color_fg ) ) return false;
				break;
			case 39: // default foreground color
				result->color_fg = Color::vga_colors[ 7 ];
				break;
			case 40: case 41: case 42: case 43: [[fallthrough]]; // 3-bit background
			case 44: case 45: case 46: case 47:
				result->color_bg = Color::vga_colors[ cmd - 40 ];
				break;
			case 48:
				if ( !parse_8bit_or_24bit_color( &iter, &result->color_bg ) ) return false;
				break;
			case 49: // default background color
				result->color_bg = Color::vga_colors[ 7 ];
				break;
			case 50: // disable proportional font
				result->font_proportional = 0;
				break;
			case 51: // framed
				result->framed = 1;
				break;
			case 52: // encircled
				result->encircled = 1;
				break;
			case 53: // overline
				result->line_over = 1;
				break;
			case 54: // not framed or encircled
				result->framed = result->encircled = 0;
				break;
			case 55: // not overlined
				result->line_over = 0;
				break;
			// (no 56, 57)
			case 58: // set underline color
				if ( !parse_8bit_or_24bit_color( &iter, &result->color_lines ) ) return false;
				break;
			case 59: // default underline color
				result->color_lines = Color::vga_colors[ 7 ];
				break;
			case 60: // ideogram right-side line
				result->lines_right = 1;
				break;
			case 61: // ideogram right-side double line
				result->lines_right = 2;
				break;
			case 62: // ideogram left-side line
				result->lines_left = 1;
				break;
			case 63: // ideogram left-side double line
				result->lines_left = 2;
				break;
			case 64: // ideogram stress mark
				result->stress_mark = 1;
				break;
			case 65: // reset 60--64
				result->lines_left = result->lines_right = result->stress_mark = 0;
				break;
			// ( no 66--72 )
			case 73: // superscript
				result->font_sup = 1;
				break;
			case 74: // subscript
				result->font_sub = 1;
				break;
			case 75: // not superscript or subscript
				result->font_sup = result->font_sub = 0;
				break;
			// ( no 76--89 )
			case 90: case 91: case 92: case 93: [[fallthrough]]; // 3-bit foreground (bright)
			case 94: case 95: case 96: case 97:
				result->color_fg = Color::vga_colors[ cmd - 90 + 8 ];
				break;
			case 100: case 101: case 102: case 103: [[fallthrough]]; // 3-bit background (bright)
			case 104: case 105: case 106: case 107:
				result->color_bg = Color::vga_colors[ cmd - 100 + 8 ];
				break;
			// ( no 108--255 )
			default:
				return false;
		}
	}
	if ( conceal ) [[unlikely]] result->color_fg=result->color_bg;
	if ( reverse ) [[unlikely]] std::swap( result->color_fg, result->color_bg );
	return true;
}

void CharMatrix::resize( uint32 w, uint32 h )
{
	if ( w==size[0] && h==size[1] ) [[unlikely]] return;

	Char defl_char = { '\0', curr_attribs };

	Char* new_buffer = new Char[ h * w ];
	uint32 i, j=0;
	if ( buffer != nullptr )
	{
		uint32 hi[2] = { std::min(size[0],w), std::min(size[1],h) };
		for     ( j=0; j<hi  [1]; ++j )
		{
			for ( i=0; i<hi  [0]; ++i ) new_buffer[ j*w + i ] = buffer[ j*size[0] + i ];
			for (    ; i<size[0]; ++i ) new_buffer[ j*w + i ] = defl_char;
		}

		delete[] buffer;
	}
	for (    ; j<h; ++j )
	for ( i=0; i<w; ++i )
	{
		new_buffer[ j*w + i ] = defl_char;
	}

	size[0] = w;
	size[1] = h;
	buffer = new_buffer;
}

void CharMatrix::clear() noexcept
{
	Char* ptr = buffer;
	Char defl_char = { '\0', curr_attribs };
	for ( uint32 j=0; j<size[1]; ++j )
	for ( uint32 i=0; i<size[0]; ++i )
	{
		*(ptr++) = defl_char;
	}
}

void CharMatrix::scroll( int32 lines/*=1*/ )
{
	if ( lines == 0 ) return;

	bool is_pos = lines > 0;
	if ( !is_pos ) lines=-lines;
	if ( lines >= size[1] ) return clear();

	Char defl_char = { '\0', curr_attribs };

	int32 j;
	if ( is_pos )
	{
		j = 0;
		for ( j=0; j<(int32)(size[1]-lines); ++j )
		{
			std::memcpy(
				buffer+j*size[0], buffer+(j+lines)*size[0],
				size[0]*sizeof(Char)
			);
		}
		for (           ; j<size[1]; ++j )
		for ( uint32 i=0; i<size[0]; ++i )
		{
			buffer[ j*size[0] + i ] = defl_char;
		}
	}
	else
	{
		for ( j=(int32)(size[1]-1); j>=lines; --j )
		{
			std::memcpy(
				buffer+j*size[0], buffer+(j-lines)*size[0],
				size[0]*sizeof(Char)
			);
		}
		for (           ; j>=0      ; --j )
		for ( uint32 i=0; i< size[0]; ++i )
		{
			buffer[ j*size[0] + i ] = defl_char;
		}
	}
}



BasicTextUI::BasicTextUI( uint32 buffer_w,uint32 buffer_h, uint32 view_h ) noexcept :
	char_buffer( buffer_w, buffer_h )
{
	__moss_assert( buffer_w>0 && buffer_h>0, "Buffer size must be positive!" );

	__moss_assert( view_h>0, "View height must be positive!" );
	__moss_assert( buffer_h>=view_h, "Buffer height must be at least the view height!" );
	_view.height = view_h;
	_view.row_offset = char_buffer.size[1] - _view.height;

	// Any changes (including the start) will trigger re-rendering
	char_buffer.curr_attribs.is_dirty = 1;
	render_set_all_view_dirty();
}

void BasicTextUI::render_subrow_range(
	uint32 col_ind_start,uint32 col_ind_end, uint32 row_ind, unichar ch
) noexcept {
	// TODO: optimize like in `.render_all(⋯)`
	for ( uint32 col_ind=col_ind_start; col_ind<col_ind_end; ++col_ind )
	{
		render_at( {col_ind,row_ind}, ch );
	}
}
void BasicTextUI::render_rows( uint32 row_ind_start,uint32 row_ind_end, unichar ch ) noexcept
{
	// TODO: optimize like in `.render_all(⋯)`
	for ( uint32 row_ind=row_ind_start; row_ind<row_ind_end; ++row_ind )
	{
		render_subrow_range( 0,char_buffer.size[0], row_ind, ch );
	}
}
void BasicTextUI::render_all_view  ( unichar ch ) noexcept
{
	Char* ptr = &get_at({ 0, 0 });
	Char repl = { ch, char_buffer.curr_attribs };

	for ( uint32 j=0; j<_view.height       ; ++j )
	for ( uint32 i=0; i<char_buffer.size[0]; ++i )
	{
		*(ptr++) = repl;
	}
}
void BasicTextUI::render_all_buffer( unichar ch ) noexcept
{
	Char* ptr = char_buffer.buffer;
	Char repl = { ch, char_buffer.curr_attribs };

	for ( uint32 j=0; j<char_buffer.size[1]; ++j )
	for ( uint32 i=0; i<char_buffer.size[0]; ++i )
	{
		*(ptr++) = repl;
	}
}
void BasicTextUI::render_set_all_view_dirty() noexcept
{
	Char* ptr = &get_at({ 0, 0 });

	for ( uint32 j=0; j<_view.height       ; ++j )
	for ( uint32 i=0; i<char_buffer.size[0]; ++i )
	{
		(ptr++)->attribs.is_dirty = 1;
	}
}

bool BasicTextUI::cursor_move_absolute( CharInd const& inds_col_row ) noexcept
{
	bool ret = true;

	uint32 col_ind = inds_col_row[0];
	uint32 row_ind = inds_col_row[1];
	if ( col_ind >= char_buffer.size[0] ) [[unlikely]] { ret=false; col_ind=char_buffer.size[0]-1; }
	if ( row_ind >= _view.height        ) [[unlikely]] { ret=false; row_ind=_view.height       -1; }

	_cursor_ind = { col_ind, row_ind };

	return ret;
}
bool BasicTextUI::cursor_move_relative( int32 delx, int32 dely      ) noexcept
{
	bool ret = true;

	int64 tmpx = (int64)delx + (int64)_cursor_ind[0];
	int64 tmpy = (int64)dely + (int64)_cursor_ind[1];
	if      ( tmpx <  0                   ) [[unlikely]] { ret=false; tmpx=0                    ; }
	else if ( tmpx >= char_buffer.size[0] ) [[unlikely]] { ret=false; tmpx=char_buffer.size[0]-1; }
	if      ( tmpy <  0                   ) [[unlikely]] { ret=false; tmpy=0                    ; }
	else if ( tmpy >= _view.height        ) [[unlikely]] { ret=false; tmpy=_view.height       -1; }

	_cursor_ind[0] = (uint32)tmpx;
	_cursor_ind[1] = (uint32)tmpy;

	return ret;
}

void BasicTextUI::clear_screen() noexcept
{
	char_buffer.scroll( _cursor_ind[1] );
	_cursor_ind[1] = 0;
}

void BasicTextUI::_esc_abort() noexcept
{
	__moss_assert( !_esc_seq.empty(), "Already not in escape!" );
	bell();
	_esc_seq.clear();
}

[[nodiscard]] bool BasicTextUI::_esc_csi() noexcept
{
	__moss_assert_impl(
		_esc_seq.size()>=3 &&
		_esc_seq[0]=='\e' && _esc_seq[1]=='[' &&
		_esc_seq.back()>=0x40 && _esc_seq.back()<=0x7e
	);

	// Extract final byte
	char final_byte = _esc_seq.back();
	_esc_seq.pop_back();

	// Skip 'CSI' "\e["
	_esc_seq.erase( 0, 2 );

	// Some kind of private sequence, not supported
	if ( _esc_seq.size()>0 && _esc_seq[0]=='?' ) return false;

	// The other supported kinds have a semicolon-separated list of integer arguments
	std::vector<int32> args = _parse_semi_int_seq( _esc_seq );
	if ( args.empty() ) return false; // Something other than semicolon-separated integers

	switch ( final_byte )
	{
		#define MOSS_CHECK_1_ARG( DEFAULT_VAL )\
			if ( args.size() != 1 ) return false;\
			if ( args[0] == -1 ) args[0]=DEFAULT_VAL;

		case 'A': // Move cursor up    by 1 (or by int given in param bytes), if not already at edge
			MOSS_CHECK_1_ARG( 1 )
			(void)cursor_move_relative( 0, -(int32)args[0] );
			break;
		case 'B': // Move cursor down  by 1 (or by int given in param bytes), if not already at edge
			MOSS_CHECK_1_ARG( 1 )
			(void)cursor_move_relative( 0, (int32)args[0] );
			break;
		case 'C': // Move cursor right by 1 (or by int given in param bytes), if not already at edge
			MOSS_CHECK_1_ARG( 1 )
			(void)cursor_move_relative( (int32)args[0], 0 );
			break;
		case 'D': // Move cursor left  by 1 (or by int given in param bytes), if not already at edge
			MOSS_CHECK_1_ARG( 1 )
			(void)cursor_move_relative( -(int32)args[0], 0 );
			break;
		case 'E': // Move cursor to beginning of line, 1 (or int param) line(s) down
			MOSS_CHECK_1_ARG( 1 )
			(void)cursor_move_relative( -(int32)_cursor_ind[0], (int32)args[0] );
			break;
		case 'F': // Move cursor to beginning of line, 1 (or int param) line(s) up
			MOSS_CHECK_1_ARG( 1 )
			(void)cursor_move_relative( -(int32)_cursor_ind[0], -(int32)args[0] );
			break;
		case 'G': // Move cursor to column 1 (or int param)
			if ( args.size()!=1 || args[0]==0 ) return false;
			if ( args[0] == -1 ) args[0]=1;
			(void)cursor_move_absolute({ (uint32)args[0]-1, _cursor_ind[1] });
			break;
		case 'H': SET_CURSOR_POS: // Move cursor to row and column (defaults 1)
			if ( args.size()!=2 || args[0]==0 || args[1]==0 ) return false;
			if ( args[0] == -1 ) args[0]=1;
			if ( args[1] == -1 ) args[1]=1;
			(void)cursor_move_absolute({ (uint32)args[1]-1, (uint32)args[0]-1 });
			break;
		case 'J': // Erase in display
			MOSS_CHECK_1_ARG( 0 )
			char_buffer.curr_attribs = CharAttribs::get_default();
			switch ( args[0] )
			{
				case 0: //Clear from cursor to end of screen
					render_rows( _cursor_ind[1]+1,_view.height, '\0' );
					goto ERASE_ROW_AFTER_CURSOR;
				case 1: //Clear from cursor to beginning of screen
					if ( _cursor_ind[1] > 0 ) render_rows( 0,_cursor_ind[1]-1, '\0' );
					goto ERASE_ROW_BEFORE_CURSOR;
				case 2: //Clear entire screen and move cursor to top left
					render_all_view  ( '\0' );
					_cursor_ind = { 0, 0 };
					break;
				case 3: //Clear screen and buffer and move cursor to top left
					render_all_buffer( '\0' );
					_cursor_ind = { 0, 0 };
					break;
				default:
					return false;
			}
			break;
		case 'K': // Erase in line without moving cursor: 0=to end (default), 1=to beginning, 2=all
			MOSS_CHECK_1_ARG( 0 )
			switch ( args[0] )
			{
				case 0: ERASE_ROW_AFTER_CURSOR:
					render_subrow_range(
						_cursor_ind[0], char_buffer.size[0],
						_cursor_ind[1],
						'\0'
					);
					break;
				case 1: ERASE_ROW_BEFORE_CURSOR:
					render_subrow_range(
						0, _cursor_ind[0],
						_cursor_ind[1],
						'\0'
					);
					break;
				case 2:
					render_subrow_range( 0,char_buffer.size[0], _cursor_ind[1], '\0' );
					break;
				default:
					return false;
			}
			break;
		case 'S': // Scroll up   1 (or int param)
			MOSS_CHECK_1_ARG( 1 )
			char_buffer.scroll( -args[0] );
			break;
		case 'T': // Scroll down 1 (or int param)
			MOSS_CHECK_1_ARG( 1 )
			char_buffer.scroll( args[0] );
			break;
		case 'f': // Sets cursor position but different somehow
			goto SET_CURSOR_POS;
		case 'm': // Graphics
			MOSS_CHECK_1_ARG( 0 )
			if ( !CharMatrix::apply_sgr_esc_seq(
				char_buffer.curr_attribs, args, &char_buffer.curr_attribs
			) ) return false;
			break;
		case 'i': // AUX port on or off, not supported
			return false;
		case 'n': // Report cursor position, not supported
			if ( args.size()!=1 && args[0]!=6 ) return false;
			return false;
		default: // Invalid / unknown
			return false;

		#undef MOSS_CHECK_1_ARG
	}

	return true;
}
[[nodiscard]] bool BasicTextUI::_esc_add( char ch ) noexcept
{
	_esc_seq += ch;
	if      ( _esc_seq.size() == 1 )
	{
		__moss_assert_impl( ch == '\e' );
	}
	else if ( _esc_seq.size() == 2 )
	{
		switch ( ch )
		{
			// Fe escape sequences
			case '[' : // Control sequence introducer
				break;

			#if 0
			case 'N' : // Select "G2" character set for next char
			case 'O' : // Select "G3" character set for next char
			case 'P' : // Device control string
			case '\\': // String terminator
			case ']' : // OS command
			case 'X' : case '^': case '_': // String control sequences
			#endif

			// Unsupported or unknown; abort escape sequence
			default:
				goto INVALID;
		}
	}
	else
	{
		// 'CSI' code; see comment at top of file and `._esc_csi()`.
		__moss_assert_impl( _esc_seq.size()>=3 && _esc_seq[0]=='\e' && _esc_seq[1]=='[' );

		if      ( ch>=0x30 && ch<=0x3f ) [[likely  ]] {} // parameter byte
		else if ( ch>=0x20 && ch<=0x2f ) [[unlikely]] {} // intermediate byte
		else if ( ch>=0x40 && ch<=0x7e ) // final byte
		{
			bool ret = _esc_csi();
			_esc_seq.clear();
			return ret;
		}
		else [[unlikely]] goto INVALID; // invalid byte
	}

	return true;

	INVALID:
	_esc_abort();
	return false;
}
bool BasicTextUI::write ( char ch                   ) noexcept
{
	switch ( ch )
	{
		//C0 control codes
		#if 1

		case '\a': //Bell ''
			bell();
			return true;

		case '\b': //Backspace ''
			return false; // Not supported

		case '\t':
			/*cursor1_offset += tab_width - cursor1_offset%tab_width;
			current.text += ch;
			break;*/
			return false; // Not supported

		case '\n':
		{
			newline();

			if ( ++_cursor_ind[1] == _view.height ) [[unlikely]]
			{
				--_cursor_ind[1];
				char_buffer.scroll();
			}
			if ( implicit_cr_on_lf ) _cursor_ind[0]=0;

			return true;
		}

		case '\f': //Form feed ''
			clear_screen();
			return true;

		case '\r':
			_cursor_ind[0] = 0;
			return true;

		#endif

		//Start escape sequence
		case '\e':
			return _esc_add( ch );

		//Non-escape character
		default:
		{
			if ( !_esc_seq.empty() ) return _esc_add( ch );

			render_at( _cursor_ind, ch );
			if ( ++_cursor_ind[0] == char_buffer.size[0] ) [[unlikely]]
			{
				_cursor_ind[0] = 0;
				if ( ++_cursor_ind[1] == _view.height ) [[unlikely]]
				{
					--_cursor_ind[1];
					char_buffer.scroll();
				}
			}
			return true;
		}
	}
}
void BasicTextUI::write ( char const* str           ) noexcept
{
	while ( *str != '\0' ) write( *(str++) );
}
// TODO: threadsafety
static BasicTextUI* _g_current;
static int _write_stream( char ch, void* ) noexcept
{
	return _g_current->write( ch ) ? ch : 0;
}
MOSS_VARFN_ATTRIB( printf, 2, 3 )
void BasicTextUI::writef( char const* fmt_cstr, ... ) noexcept
{
	va_list args;
	va_start( args, fmt_cstr );

	_g_current = this;
	std::FILE file = { MOSS_STREAM_FUNCTION, .fn={&_write_stream,nullptr} };
	std::vfprintf( &file, fmt_cstr,args );

	va_end( args );
}

void BasicTextUI::refresh_draw_row( uint32 row_ind ) noexcept
{
	Char* row_start = &get_at({ 0, row_ind });
	Char* rng_start = row_start;
	Char* row_end   = row_start + char_buffer.size[0];
	LOOP_BLOCK:
		LOOP_SKIP:
			if ( !rng_start->attribs.is_dirty )
			{
				if ( ++rng_start == row_end ) [[unlikely]] return;
				goto LOOP_SKIP;
			}

		Char* rng_end = rng_start + 1;
		LOOP_UPDATE:
			if ( rng_end<row_end && rng_end->attribs==rng_start->attribs )
			{
				++rng_end;
				goto LOOP_UPDATE;
			}

		refresh_set_draw_pos( rng_start-row_start, row_ind );
		refresh_set_draw_attribs( rng_start->attribs );
		refresh_draw_row_range( rng_start, rng_end );
		do { rng_start->attribs.is_dirty=0; } while ( ++rng_start < rng_end );

		if ( rng_end < row_end )
		{
			rng_start = rng_end;
			goto LOOP_BLOCK;
		}
}
void BasicTextUI::refresh() noexcept
{
	#if 1
		for ( uint32 j=0; j<_view.height; ++j ) refresh_draw_row(j);
	#else
		for ( uint32 j=0; j<_view.height; ++j )
		{
			refresh_set_draw_pos( 0, j );

			for ( uint32 i=0; i<char_buffer.size[0]; ++i )
			{
				Char const& styled_ch = get_at({ i, j });
				refresh_set_draw_attribs( styled_ch.attribs );
				char ch = (char)styled_ch.ch;
				std::fputc( ch=='\0'?' ':ch, stdout );
			}
		}
	#endif

	refresh_set_draw_pos( _cursor_ind[0], _cursor_ind[1] );
}



#if 0
TerminalBase::TerminalBase(
	uint32 buffer_w,uint32 buffer_h, uint32 view_h,
	size_t max_history
) noexcept
{
	history.max = max_history; // can be 0
}

void TerminalBase::set_header( std::string const& header ) noexcept
{
	//TODO: improve
	TextUI::Char* ptr = get_buffer_current_row();
	for ( size_t i=0; i<current.header.size()+current.text.size(); ++i ) _render_set_at(ptr+i,'\0');

	if ( current.write_target & 0b1'0 ) esc_abort();
	__moss_assert_impl( current.write_target == 0b0'1 );

	current.header.clear();
	current.write_target = 0b0'0;
	for ( char ch : header )
	{
		MOSS_DEBUG_ONLY(bool ret =) write( ch );
		__moss_assert_impl( ret );
	}

	__moss_assert_impl( (current.write_target&0b0'1) == 0b0'0 );
	if ( current.write_target & 0b1'0 ) esc_abort();

	current.write_target = 0b0'1;
}

void TerminalBase::_render_set_at( TextUI::Char* ptr, char ch ) noexcept
{
	__moss_assert_impl(
		ptr >= char_buffer.buffer &&
		ptr <  char_buffer.buffer + char_buffer.size[1]*char_buffer.size[0]
	);
	TextUI::Char& ref = *ptr;

	ref = { ch, attribs };
	ref.attribs.is_dirty = 1;
}
void TerminalBase::_render_write_text_clear( size_t offset, size_t count ) noexcept
{
	TextUI::Char* dst = get_buffer_current_text() + offset;
	__moss_assert_impl(
		dst       >= char_buffer.buffer &&
		dst+count <  char_buffer.buffer + char_buffer.size[1]*char_buffer.size[0]
	);
	while ( count > 0 ) { _render_set_at(dst++,'\0'); --count; }
}
void TerminalBase::_render_write_text_curr( std::vector<unichar>::iterator start_iter ) noexcept
{
	TextUI::Char* dst = get_buffer_current_text();
	__moss_assert_impl(
		dst                     >= char_buffer.buffer &&
		dst+current.text.size() <  char_buffer.buffer + char_buffer.size[1]*char_buffer.size[0] &&
		start_iter != current.text.end()
	);

	size_t offset = start_iter - current.text.begin();
	dst += offset;
	unichar const* src = current.text.data() + offset;

	while ( *src != '\0' ) _render_set_at( dst++, (char)(*(src++)) );
}
bool TerminalBase::edit_del_1_before   () noexcept
{
	__moss_assert_impl( cursor1_offset <= current.text.size() );
	if ( cursor1_offset == 0 ) return false;
	__moss_assert_impl( !current.text.empty() );

	--cursor1_offset;
	return edit_del_1_after();
}
bool TerminalBase::edit_del_1_after    () noexcept
{
	__moss_assert_impl( cursor1_offset <= current.text.size() );
	if ( cursor1_offset == current.text.size() ) return false;

	TextUI::Char* ptr = get_buffer_current_text();
	_edit_set_at( ptr+current.text.size()-1, '\0' );

	current.text.erase( current.text.begin() + cursor1_offset );

	_edit_write_text_curr( current.text.begin() + cursor1_offset );

	return true;
}
bool TerminalBase::edit_del_word_before() noexcept
{
	__moss_assert_impl( cursor1_offset <= current.text.size() );

	auto rng_begin = current.text.begin() + cursor1_offset;
	auto rng_end = rng_begin;
	//Walk backward to find the first text
	LOOP1:
		if ( rng_begin>current.text.begin() && std::isspace((char)rng_begin[-1]) )
		{
			--rng_begin;
			goto LOOP1;
		}
	//Walk through the word
	LOOP2:
		if ( rng_begin>current.text.begin() && std::isgraph((char)rng_begin[-1]) )
		{
			--rng_begin;
			goto LOOP2;
		}

	size_t erase_amt = rng_end - rng_begin;
	__moss_assert_impl( erase_amt <= cursor1_offset );
	if ( erase_amt == 0 ) return false;
	cursor1_offset -= erase_amt;

	current.text.erase( rng_begin, rng_end );

	_edit_write_text_curr( current.text.begin() + cursor1_offset );
	_edit_write_text_clear( cursor1_offset, erase_amt );

	return true;
}
bool TerminalBase::edit_del_word_after () noexcept
{
	__moss_assert_impl( cursor1_offset <= current.text.size() );
	if ( cursor1_offset == current.text.size() ) return false;

	auto rng_begin = current.text.begin() + cursor1_offset;
	auto rng_end = rng_begin;
	//Walk forward to find the first text
	LOOP1:
		if ( rng_end<current.text.end() && std::isspace((char)rng_end[0]) )
		{
			++rng_end;
			goto LOOP1;
		}
	//Walk through the word
	LOOP2:
		if ( rng_end<current.text.end() && std::isgraph((char)rng_end[0]) )
		{
			++rng_end;
			goto LOOP2;
		}

	size_t erase_amt = rng_end - rng_begin;
	__moss_assert_impl( erase_amt <= cursor1_offset );
	if ( erase_amt == 0 ) return false;

	current.text.erase( rng_begin, rng_end );

	_edit_write_text_curr( current.text.begin() + cursor1_offset );
	_edit_write_text_clear( cursor1_offset, erase_amt );

	return true;
}
bool TerminalBase::edit_del_all_before () noexcept
{
	__moss_assert_impl( cursor1_offset <= current.text.size() );
	if ( cursor1_offset == 0 ) return false;

	size_t erase_amt = cursor1_offset;
	current.text.erase( current.text.begin(), current.text.begin()+cursor1_offset );
	cursor1_offset = 0;

	_edit_write_text_curr( current.text.begin() );
	_edit_write_text_clear( current.text.size(), erase_amt );

	return true;
}
bool TerminalBase::edit_del_all_after  () noexcept
{
	__moss_assert_impl( cursor1_offset <= current.text.size() );
	if ( cursor1_offset == current.text.size() ) return false;

	_edit_write_text_clear( cursor1_offset, current.text.size()-cursor1_offset );
	current.text.erase( current.text.begin()+cursor1_offset, current.text.end() );

	return true;
}
bool TerminalBase::edit_del_all        () noexcept
{
	__moss_assert_impl( cursor1_offset <= current.text.size() );
	if ( current.text.empty() ) return false;

	_edit_write_text_clear( 0, current.text.size() );
	current.text.clear();
	cursor1_offset = 0;

	return true;
}
bool TerminalBase::edit_transpose_at() noexcept
{
	__moss_assert_impl( cursor1_offset <= current.text.size() );
	if ( cursor1_offset==0 || cursor1_offset==current.text.size() ) return false;

	std::swap(
		current.text[ cursor1_offset - 1 ],
		current.text[ cursor1_offset     ]
	);

	TextUI::Char* ptr = get_buffer_current_text() + cursor1_offset;
	std::swap( ptr[-1], ptr[0] );
	ptr[0].attribs.is_dirty = 1;
	ptr[1].attribs.is_dirty = 1;

	return true;
}




#if 0
bool TerminalBase::cursor_backward( uint32 count/*=1*/ ) noexcept
{
	if ( _screen.cursor_ind[0] < count ) return false;
	_screen.cursor_ind[0] -= count;
	render_cursor();
	return true;
}
bool TerminalBase::cursor_forward ( uint32 count/*=1*/ ) noexcept
{
	uint32 remaining = _screen.size[0]-1 - _screen.cursor_ind[0];
	if ( count > remaining ) return false;
	_screen.cursor_ind[0] += count;
	render_cursor();
	return true;
}
bool TerminalBase::cursor_up      ( uint32 count/*=1*/ ) noexcept
{
	if ( _screen.cursor_ind[1] > 0 )
	{
		--_screen.cursor_ind[1];
		render_cursor();
		return true;
	}
	else if ( _screen.ind_first_line > 0 )
	{
		--_screen.ind_first_line;
		render_all_lines_and_cursor();
		return true;
	}
	return false;
}
bool TerminalBase::cursor_down    ( uint32 count/*=1*/ ) noexcept
{
	if ( _screen.cursor_ind[1] > 0 )
	{
		--_screen.cursor_ind[1];
		render_cursor();
		return true;
	}
	else if ( _screen.ind_first_line > 0 )
	{
		--_screen.ind_first_line;
		render_all_lines_and_cursor();
		return true;
	}
	return false;
}
void TerminalBase::cursor_home() noexcept
{
}
void TerminalBase::cursor_end () noexcept
{
}
void TerminalBase::cursor_pageup  () noexcept
{
}
void TerminalBase::cursor_pagedown() noexcept
{
}
void TerminalBase::cursor_set_pos( uint32 col_ind,uint32 row_ind, bool constrain_to_text/*=false*/ ) noexcept
{
}




[[nodiscard]] LocInfo TerminalBase::get_at( uint32 col_ind, uint32 row_ind ) const noexcept
{
	__moss_assert_impl( !cmds.empty() );
	auto iter1 = cmds.cbegin() + view_cmd_ind[0];
	LOOP:
		auto iter2 = iter1;
		if ( ++iter2!=cmds.cend() && (*iter2). )
}


case '\n':
{
	cmd_exec_current();

	if ( ++cursor1_ind_y == view.height )
	{
		--cursor1_ind_y;
		char_buffer.scroll_down();
	}
	edit_del_all();

	get_buffer_current_row()

	_lines.emplace_back();
	_screen.cursor_ind[0] = 0;
	++_screen.cursor_ind[1];
	dirty = 3;

	__moss_assert_impl( _screen.cursor_ind[1] <= _screen.size[1] );
	if ( _screen.cursor_ind[1] == _screen.size[1] )
	{
		--_screen.cursor_ind[1];
		++_screen.ind_first_line;
		dirty = 7;
	}

	if ( _lines.size() > _max_lines )
	{
		_lines.erase( _lines.begin() );
		__moss_assert_impl( _lines.size() == _max_lines );
		--_screen.ind_first_line;
	}

	break;
}

#endif

#endif



}
