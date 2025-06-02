#pragma once

//#include "stdafx.hpp"
#include "color.hpp"
#include "unichar.hpp"



namespace MOSS::Term
{



/*
Matrix of styled characters
*/
class CharMatrix final
{
	public:
		using CharInd = std::array< uint32, 2 >;

		struct CharAttribs final
		{
			Color::sRGB color_fg;
			Color::sRGB color_bg;
			Color::sRGB color_lines;

			uint8 lines_left        : 2; // 0, 1, or 2
			uint8 lines_right       : 2; // 0, 1, or 2
			uint8 lines_under       : 2; // 0, 1, or 2
			uint8 line_over         : 1;
			uint8 line_through      : 1;

			uint8 font_index        : 4; // 0=default, [1,9]=alternate, 10=fraktur
			uint8 font_bold         : 1;
			uint8 font_faint        : 1;
			uint8 font_italic       : 1;
			uint8 font_sub          : 1;

			uint8 font_sup          : 1;
			uint8 font_proportional : 1;
			uint8 blink             : 2; // 0=none, 1=slow, 2=rapid
			uint8 framed            : 1;
			uint8 encircled         : 1;
			uint8 stress_mark       : 1;
			uint8 is_dirty          : 1; // for user use

			[[nodiscard]] static CharAttribs get_default();

			[[nodiscard]] bool operator==( CharAttribs const& other ) const noexcept = default;
		};
		struct Char final
		{
			unichar ch;
			CharAttribs attribs;
		};
		static_assert( sizeof(Char) == 16 );

		uint32 size[2];
		Char* buffer;

		// "Current" attributes that will be applied to operations (e.g. `.clear(⋯)` clears to
		// this, empty space in `.resize(⋯)` is filled with this).  User can set.
		CharAttribs curr_attribs = CharAttribs::get_default();

	public:
		CharMatrix( uint32 w, uint32 h ) noexcept : size{0,0},buffer(nullptr) { resize(w,h); }

		[[nodiscard]] static bool apply_sgr_esc_seq(
			CharAttribs const& attribs, std::vector<int32> const& cmds, CharAttribs* result
		) noexcept;

		void resize( uint32 w, uint32 h );

		void clear() noexcept;

		//Note not mathematical matrix order.  This is ( column, row )
		[[nodiscard]] Char const& operator[]( CharInd const& inds_col_row ) const noexcept
		{
			__moss_assert( inds_col_row[0]<size[0], "%s access out of bounds!", "column" );
			__moss_assert( inds_col_row[1]<size[1], "%s access out of bounds!", "row"    );
			return buffer[ inds_col_row[1]*size[0] + inds_col_row[0] ];
		}
		[[nodiscard]] Char      & operator[]( CharInd const& inds_col_row )       noexcept
		{
			__moss_assert( inds_col_row[0]<size[0], "%s access out of bounds!", "column" );
			__moss_assert( inds_col_row[1]<size[1], "%s access out of bounds!", "row"    );
			return buffer[ inds_col_row[1]*size[0] + inds_col_row[0] ];
		}

		void scroll( int32 lines=1 );
};



/*
A view into a `CharMatrix`, also with printing, escape-sequence, and rendering functionality.  The
view is initially at the end of the buffer.
*/
class BasicTextUI
{
	public:
		using Char        = CharMatrix::Char       ;
		using CharAttribs = CharMatrix::CharAttribs;
		using CharInd = CharMatrix::CharInd;

		// Matrix of styled characters (note: probably larger than `.view.height`)
		CharMatrix char_buffer;

		bool implicit_cr_on_lf = true;

	protected:
		// The actual subregion of the buffer that is visible
		struct
		{
			uint32 height;
			uint32 row_offset; // vertical offset into the buffer
		} _view;

		std::string _esc_seq;

		// Cursor column and row within the view.  The cursor is not visible; draw it yourself!
		CharInd _cursor_ind = { 0, 0 };

	public:
		BasicTextUI( uint32 buffer_w,uint32 buffer_h, uint32 view_h ) noexcept;

		[[nodiscard]] Char const& get_at( CharInd const& inds_col_row ) const noexcept
		{
			__moss_assert(
				inds_col_row[0]                  < char_buffer.size[0] &&
				inds_col_row[1]+_view.row_offset < char_buffer.size[1],
				"Invalid cursor position!"
			);
			return char_buffer[{ inds_col_row[0], inds_col_row[1]+_view.row_offset }];
		}
		[[nodiscard]] Char      & get_at( CharInd const& inds_col_row )       noexcept
		{
			__moss_assert(
				inds_col_row[0]                  < char_buffer.size[0] &&
				inds_col_row[1]+_view.row_offset < char_buffer.size[1],
				"Invalid cursor position!"
			);
			return char_buffer[{ inds_col_row[0], inds_col_row[1]+_view.row_offset }];
		}

		void render_at( CharInd const& inds_col_row, unichar ch ) noexcept
		{
			Char& dst = get_at( inds_col_row );
			dst.ch = ch;
			dst.attribs = char_buffer.curr_attribs;
		}
		void render_subrow_range(
			uint32 col_ind_start,uint32 col_ind_end, uint32 row_ind, unichar ch
		) noexcept;
		void render_rows( uint32 row_ind_start,uint32 row_ind_end, unichar ch ) noexcept;
		void render_all_view  ( unichar ch ) noexcept;
		void render_all_buffer( unichar ch ) noexcept;
		void render_set_all_view_dirty() noexcept;

		bool cursor_move_absolute( CharInd const& inds_col_row ) noexcept;
		bool cursor_move_relative( int32 delx, int32 dely      ) noexcept;

		void clear_screen() noexcept;

	private:
		void _esc_abort() noexcept;

		[[nodiscard]] bool _esc_csi() noexcept;
		[[nodiscard]] bool _esc_add( char ch ) noexcept;
	public:
		bool write ( char ch                   ) noexcept;
		void write ( char const* str           ) noexcept;
		//Not threadsafe
		MOSS_VARFN_ATTRIB( printf, 2, 3 )
		void writef( char const* fmt_cstr, ... ) noexcept;

		virtual void debug_print_str( char const* str ) const noexcept {}

		virtual void bell() noexcept {}
		virtual void newline() noexcept {}

		virtual void refresh_set_draw_pos( uint32 col_ind, uint32 row_ind ) noexcept {}
		virtual void refresh_set_draw_attribs( CharAttribs const& attribs ) noexcept {}
		virtual void refresh_draw_row_range( Char const* rng_start, Char const* rng_end ) noexcept {}
		void refresh_draw_row( uint32 row_ind ) noexcept;
		void refresh() noexcept;
};



#if 0
class TerminalBase
{
	public:
		
		

		/*
		Record of commands the user actually executed.  Using up / down changes `.load_ind` and then
		loads the `.load_ind`th element of `.entries`.  Executing a command appends to `.entries`,
		deletes old out of `.entries` if it's larger than `.max`, and finally sets `.load_ind` to
		`.entries.size()-1`.
		*/
		struct
		{
			std::vector< std::string > entries; // TODO: `deque`
			size_t max;
			size_t load_ind = 0;
		} history;

		// The current header (e.g. "$> ") and text the user is in the process of typing (e.g. "ls")
		struct
		{
			std::vector<unichar> header;
			std::vector<unichar> text;
			std::string escape;
			unsigned write_target = 0b0'0; // '?0'=header, '?1'=text, '1?'=escape
		} current;

		// Text-editing cursor (cursor 2 is for selection, not implemented)
		uint16 cursor1_ind_y  = 0; // row index of the cursor within the view
		size_t cursor1_offset = 0; // char offset within `text`

		// Config state
		uint16 tab_width = 8; // ANSI-required default?
		bool mode_insert = false;
		bool mode_select = false;

	protected:
		TerminalBase(
			uint32 buffer_w,uint32 buffer_h, uint32 view_h,
			size_t max_history
		) noexcept;
		virtual ~TerminalBase() = default;

		[[nodiscard]] TextUI::Char const* get_buffer_view        () const noexcept
		{
			return char_buffer.buffer + view.row_offset*char_buffer.size[0];
		}
		[[nodiscard]] TextUI::Char      * get_buffer_view        ()       noexcept
		{
			return char_buffer.buffer + view.row_offset*char_buffer.size[0];
		}
		[[nodiscard]] TextUI::Char const* get_buffer_current_row () const noexcept
		{
			return get_buffer_view() + cursor1_ind_y*char_buffer.size[0];
		}
		[[nodiscard]] TextUI::Char      * get_buffer_current_row ()       noexcept
		{
			return get_buffer_view() + cursor1_ind_y*char_buffer.size[0];
		}
		[[nodiscard]] TextUI::Char const* get_buffer_current_text() const noexcept
		{
			return get_buffer_current_row() + current.header.size();
		}
		[[nodiscard]] TextUI::Char      * get_buffer_current_text()       noexcept
		{
			return get_buffer_current_row() + current.header.size();
		}

		void set_header( std::string const& header ) noexcept; // TODO: `std::string_view`

	private:
		void _render_set_at( TextUI::Char* ptr, char ch ) noexcept;
		void _render_write_text_clear( size_t offset, size_t count ) noexcept;
		void _render_write_text_curr( std::vector<unichar>::iterator start_iter ) noexcept;

	public:
		bool edit_del_1_before   () noexcept;
		bool edit_del_1_after    () noexcept;
		bool edit_del_word_before() noexcept;
		bool edit_del_word_after () noexcept;
		bool edit_del_all_before () noexcept;
		bool edit_del_all_after  () noexcept;
		bool edit_del_all        () noexcept;
		bool edit_transpose_at() noexcept;
		void edit_toggle_insert_mode() noexcept { mode_insert = !mode_insert; }

		bool write ( char ch                   ) noexcept;
		void write ( char const* str           ) noexcept;
		MOSS_VARFN_ATTRIB( printf, 2, 3 )
		void writef( char const* fmt_cstr, ... ) noexcept;

		virtual void bell() noexcept {}

		virtual void cmd_exec_current() noexcept {}
		virtual void cmd_terminate   () noexcept {}



		#if 0
	public:
		[[nodiscard]] std::string const& get_last_line() const noexcept { return _lines.back(); }
		[[nodiscard]] std::string      & get_last_line()       noexcept { return _lines.back(); }

		void clear_buffer() noexcept;
		void clear_screen() noexcept;

	private:
		[[nodiscard]] bool _on_editable_line() noexcept;
	public:
		

		bool cursor_backward( uint32 count=1 ) noexcept;
		bool cursor_forward ( uint32 count=1 ) noexcept;
		bool cursor_up      ( uint32 count=1 ) noexcept;
		bool cursor_down    ( uint32 count=1 ) noexcept;
		void cursor_home() noexcept;
		void cursor_end () noexcept;
		void cursor_pageup  () noexcept;
		void cursor_pagedown() noexcept;
		void cursor_set_pos( uint32 col_ind,uint32 row_ind, bool constrain_to_text=false ) noexcept;

		virtual void render_all_lines_and_cursor() noexcept {}
		virtual void render_curr_line_and_cursor() noexcept {}
		virtual void render_cursor              () noexcept {}

	private:
		[[nodiscard]] bool _handle_escseq_csi() noexcept;
		[[nodiscard]] bool _handle_escseq_add_ch( char ch ) noexcept;
	public:
		#endif
};
#endif



}
