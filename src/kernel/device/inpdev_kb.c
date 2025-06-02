#include "inpdev.h"

#include "inpdev_kb_defs.autogen.h"



#define MOSS_PRESS( BTN )\
	inpdevs_got_btn_press( dev_id, BTN )
#define MOSS_RELEASE( BTN )\
	inpdevs_got_btn_release( dev_id, BTN )
#define MOSS_PRESS_AND_RELEASE( BTN )\
	inpdevs_got_btn_press_and_release( dev_id, BTN )

MOSS_ND static bool _parse_set1_helper_make ( DEVID dev_id, uint8_t const** ptr_ptr )
{
	uint8_t const* ptr = *ptr_ptr;
	if ( ptr[0] != 0xe0 ) return false;

	INPDEV_BTN inpdev_btn;
	switch ( ptr[1] )
	{
		case 0x52: inpdev_btn=INPDEV_BTN_Insert     ; break;
		case 0x53: inpdev_btn=INPDEV_BTN_Delete     ; break;
		case 0x4b: inpdev_btn=INPDEV_BTN_LeftArrow  ; break;
		case 0x47: inpdev_btn=INPDEV_BTN_Home       ; break;
		case 0x4f: inpdev_btn=INPDEV_BTN_End        ; break;
		case 0x48: inpdev_btn=INPDEV_BTN_UpArrow    ; break;
		case 0x50: inpdev_btn=INPDEV_BTN_DownArrow  ; break;
		case 0x49: inpdev_btn=INPDEV_BTN_PageUp     ; break;
		case 0x51: inpdev_btn=INPDEV_BTN_PageDown   ; break;
		case 0x4d: inpdev_btn=INPDEV_BTN_RightArrow ; break;
		case 0x35: inpdev_btn=INPDEV_BTN_NumpadSlash; break;
		default: return false;
	}

	*ptr_ptr += 2;
	MOSS_PRESS( inpdev_btn );

	return true;
}
MOSS_ND static bool _parse_set1_helper_break( DEVID dev_id, uint8_t const** ptr_ptr )
{
	uint8_t const* ptr = *ptr_ptr;
	if ( ptr[0] != 0xe0 ) return false;

	INPDEV_BTN inpdev_btn;
	switch ( ptr[1] )
	{
		case 0xd2: inpdev_btn=INPDEV_BTN_Insert     ; break;
		case 0xd3: inpdev_btn=INPDEV_BTN_Delete     ; break;
		case 0xcb: inpdev_btn=INPDEV_BTN_LeftArrow  ; break;
		case 0xc7: inpdev_btn=INPDEV_BTN_Home       ; break;
		case 0xcf: inpdev_btn=INPDEV_BTN_End        ; break;
		case 0xc8: inpdev_btn=INPDEV_BTN_UpArrow    ; break;
		case 0xd0: inpdev_btn=INPDEV_BTN_DownArrow  ; break;
		case 0xc9: inpdev_btn=INPDEV_BTN_PageUp     ; break;
		case 0xd1: inpdev_btn=INPDEV_BTN_PageDown   ; break;
		case 0xcd: inpdev_btn=INPDEV_BTN_RightArrow ; break;
		case 0xb5: inpdev_btn=INPDEV_BTN_NumpadSlash; break;
		default: return false;
	}

	*ptr_ptr += 2;
	MOSS_RELEASE( inpdev_btn );

	return true;
}
MOSS_ND unsigned inpdev_kb_parse_ps2_sc_set1( DEVID dev_id, uint8_t const buf[6] )
{
	if      ( buf[0] <=   88 ) // make: "<byte>"
	{
		if ( buf[0] == 0x2b )
		{
			// This scancode is assigned to different keys in different physical layouts, so we have
			// to handle it separately
			struct InpDev_Keyboard const* kb = inpdev_kb_get( dev_id );
			MOSS_PRESS( kb->is_ansi_like?INPDEV_BTN_Backslash:INPDEV_BTN_Other76 );
			return 1;
		}
		INPDEV_BTN inpdev_btn = _sc_set1_table_make1byte[ buf[0] ];
		if ( inpdev_btn != INV )
		{
			MOSS_PRESS( inpdev_btn );
			return 1;
		}
	}
	else if ( buf[0] <=  216 ) // break: "<byte>"
	{
		if ( buf[1]>=129 && buf[1]<=217 )
		{
			if ( buf[1] == 0xab ) // (as above)
			{
				struct InpDev_Keyboard const* kb = inpdev_kb_get( dev_id );
				MOSS_RELEASE( kb->is_ansi_like?INPDEV_BTN_Backslash:INPDEV_BTN_Other76 );
				return 2;
			}
			INPDEV_BTN inpdev_btn = _sc_set1_table_break1byte[ buf[0] ];
			if ( inpdev_btn != INV )
			{
				MOSS_RELEASE( inpdev_btn );
				return 2;
			}
		}
	}
	else if ( buf[0] == 0xe0 )
	{
		// Some simple(r) cases
		if      ( buf[1]>=16 && buf[1]<=126 ) // make: "0xe0 <byte>"
		{
			if (
				/* buf[0]==0xe0 && */ buf[1]==0x37
			) {
				MOSS_PRESS( INPDEV_BTN_PrintScreen );
				return 2;
			}
			else if (
				/* buf[0]==0xe0 && */ buf[1]==0x2a && buf[2]==0xe0 && buf[3]==0x37
			) {
				MOSS_PRESS( INPDEV_BTN_PrintScreen ); // PrintScreen
				return 4;
			}
			else if (
				/* buf[0]==0xe0 && */ buf[1]==0x46 && buf[2]==0xe0 && buf[3]==0xc6
			) {
				MOSS_PRESS_AND_RELEASE( INPDEV_BTN_PauseBreak ); // Ctrl + PauseBreak
				return 4;
			}

			INPDEV_BTN inpdev_btn = _sc_set1_table_make2byte[ buf[1] - 16 ];
			if ( inpdev_btn != INV )
			{
				MOSS_PRESS( inpdev_btn );
				return 2;
			}
		}
		else if ( buf[1]>=144 && buf[1]<=237 ) // break: "0xe0 <byte>"
		{
			if ( buf[1] != 0xb7 )
			{
				INPDEV_BTN inpdev_btn = _sc_set1_table_break2byte[ buf[1] - 144 ];
				if ( inpdev_btn != INV )
				{
					MOSS_PRESS( inpdev_btn );
					return 2;
				}
			}
			else
			{
				/*
				If we get a PrintScreen break code, need to wait to see if there's a suffix.  The
				reason is that the Ctrl/Shift version is a prefix of the base version:
					e0 b7         (Ctrl/Shift)
					e0 b7 e0 aa   (base)
				So, if we see the former, have to wait to ensure we don't see the latter, lest we
				leave some bytes dangling.
				*/
				if      ( buf[2]==0x00                 ) return 0;
				else if ( buf[2]==0xe0 && buf[3]==0x00 ) return 0;

				if (
					/* buf[0]==0xe0 && buf[1]==0xb7 && */
					buf[2]==0xe0 && buf[3]==0xaa
				) {
					MOSS_RELEASE( INPDEV_BTN_PrintScreen );
					return 4;
				}

				MOSS_RELEASE( INPDEV_BTN_PrintScreen );
				return 2;
			}
		}

		// Probably IBM key numbers 75--89, 95
		bool got;
		uint8_t const* ptr = buf;
		if      ( ptr[1] == 0x2a ) // Num-Locked 75--89 make code
		{
			ptr += 2;
			got = _parse_set1_helper_make( dev_id, &ptr );
		}
		else if ( ptr[1] == 0xb6 ) // Right Shift make code
		{
			ptr += 2;
			if ( ptr[0]==0xe0 && ptr[1]==0xaa ) ptr+=2; // Both shifts
			got = _parse_set1_helper_make( dev_id, &ptr );
		}
		else if ( ptr[1] == 0xaa ) // Left Shift make code
		{
			ptr += 2;
			got = _parse_set1_helper_make( dev_id, &ptr );
		}
		else // A make or a break code
		{
			got = _parse_set1_helper_make( dev_id, &ptr );
			if ( got ) goto DONE;

			got = _parse_set1_helper_break( dev_id, &ptr );
			if ( got && ptr[0]==0xe0 )
			{
				if ( ptr[1] == 0xaa ) ptr+=2; // Num-Locked 75--89 break code
				else
				{
					if ( ptr[1] == 0x2a ) ptr+=2; // Left Shift
					if ( ptr[0]==0xe0 && ptr[1]==0x36 ) ptr+=2; // Right Shift
				}
			}
		}

		DONE:
		return got ? (int)(ptr-buf) : 0;
	}
	else if (
		buf[0]==0xe1 && buf[1]==0x1d && buf[2]==0x45 && buf[3]==0xe1 && buf[4]==0x9d && buf[5]==0xc5
	) {
		MOSS_PRESS_AND_RELEASE( INPDEV_BTN_PauseBreak ); // PauseBreak
		return 6;
	}

	return 0;
}

MOSS_ND static bool _parse_set2_helper_code( uint8_t byte, INPDEV_BTN* inpdev_btn )
{
	switch ( byte )
	{
		case 0x70: *inpdev_btn=INPDEV_BTN_Insert     ; break;
		case 0x71: *inpdev_btn=INPDEV_BTN_Delete     ; break;
		case 0x6b: *inpdev_btn=INPDEV_BTN_LeftArrow  ; break;
		case 0x6c: *inpdev_btn=INPDEV_BTN_Home       ; break;
		case 0x69: *inpdev_btn=INPDEV_BTN_End        ; break;
		case 0x75: *inpdev_btn=INPDEV_BTN_UpArrow    ; break;
		case 0x72: *inpdev_btn=INPDEV_BTN_DownArrow  ; break;
		case 0x7d: *inpdev_btn=INPDEV_BTN_PageUp     ; break;
		case 0x7a: *inpdev_btn=INPDEV_BTN_PageDown   ; break;
		case 0x74: *inpdev_btn=INPDEV_BTN_RightArrow ; break;
		case 0x4a: *inpdev_btn=INPDEV_BTN_NumpadSlash; break;
		default: return false;
	}
	return true;
}
MOSS_ND_INLINE static bool _parse_set2_helper_make ( DEVID dev_id, uint8_t const** ptr_ptr )
{
	uint8_t const* ptr = *ptr_ptr;
	if ( ptr[0] != 0xe0 ) return false;

	INPDEV_BTN inpdev_btn;
	if ( !_parse_set2_helper_code(ptr[1],&inpdev_btn) ) return false;

	*ptr_ptr += 2;
	MOSS_PRESS( inpdev_btn );

	return true;
}
MOSS_ND_INLINE static bool _parse_set2_helper_break( DEVID dev_id, uint8_t const** ptr_ptr )
{
	uint8_t const* ptr = *ptr_ptr;
	if ( ptr[0]!=0xe0 || ptr[1]!=0xf0 ) return false;

	INPDEV_BTN inpdev_btn;
	if ( !_parse_set2_helper_code(ptr[2],&inpdev_btn) ) return false;

	*ptr_ptr += 3;
	MOSS_RELEASE( inpdev_btn );

	return true;
}
MOSS_ND unsigned inpdev_kb_parse_ps2_sc_set2( DEVID dev_id, uint8_t const buf[8] )
{
	if      ( buf[0] <= 132 ) // make: "<byte>"
	{
		if ( buf[0] == 0x5d )
		{
			// This scancode is assigned to different keys in different physical layouts, so we have
			// to handle it separately
			struct InpDev_Keyboard const* kb = inpdev_kb_get( dev_id );
			MOSS_PRESS( kb->is_ansi_like?INPDEV_BTN_Backslash:INPDEV_BTN_Other76 );
			return 1;
		}
		INPDEV_BTN inpdev_btn = _sc_set2_table_1byte[ buf[0] ];
		if ( inpdev_btn != INV )
		{
			MOSS_PRESS( inpdev_btn );
			return 1;
		}
	}
	else if ( buf[0] == 0xf0 ) // break: "0xf0 <byte>"
	{
		if ( buf[1] <= 132 )
		{
			if ( buf[1] == 0x5d ) // (as above)
			{
				struct InpDev_Keyboard const* kb = inpdev_kb_get( dev_id );
				MOSS_RELEASE( kb->is_ansi_like?INPDEV_BTN_Backslash:INPDEV_BTN_Other76 );
				return 2;
			}
			INPDEV_BTN inpdev_btn = _sc_set2_table_1byte[ buf[1] ];
			if ( inpdev_btn != INV )
			{
				MOSS_RELEASE( inpdev_btn );
				return 2;
			}
		}
	}
	else if ( buf[0] == 0xe0 )
	{
		// Some simple(r) cases
		if      ( buf[1]>=16 && buf[1]<=126 ) // make: "0xe0 <byte>"
		{
			if (
				/* buf[0]==0xe0 && */ buf[1]==0x7c
			) {
				MOSS_PRESS( INPDEV_BTN_PrintScreen );
				return 2;
			}
			else if (
				/* buf[0]==0xe0 && */ buf[1]==0x12 && buf[2]==0xe0 && buf[3]==0x7c
			) {
				MOSS_PRESS( INPDEV_BTN_PrintScreen );
				return 4;
			}
			else if (
				/* buf[0]==0xe0 && */ buf[1]==0x7e && buf[2]==0xe0 && buf[3]==0xf0 && buf[4]==0x7e
			) {
				MOSS_PRESS_AND_RELEASE( INPDEV_BTN_PauseBreak ); // Ctrl + PauseBreak
				return 5;
			}

			INPDEV_BTN inpdev_btn = _sc_set2_table_2byte[ buf[1] - 16 ];
			if ( inpdev_btn != INV )
			{
				MOSS_PRESS( inpdev_btn );
				return 2;
			}
		}
		else if ( buf[1] == 0xf0 )
		{
			if ( buf[2]>=16 && buf[2]<=126 ) // break: "0xe0 0xf0 <byte>"
			{
				if ( buf[2] != 0x7c )
				{
					INPDEV_BTN inpdev_btn = _sc_set2_table_2byte[ buf[2] - 16 ];
					if ( inpdev_btn != INV )
					{
						MOSS_RELEASE( inpdev_btn );
						return 3;
					}
				}
				else // PrintScreen break: "e0 f0 7c[ e0 f0 12]"
				{
					/*
					If we get a PrintScreen break code, need to wait to see if there's a suffix.
					The reason is that the Ctrl/Shift version is a prefix of the base version:
						e0 f0 7c            (Ctrl/Shift)
						e0 f0 7c e0 f0 12   (base)
					So, if we see the former, have to wait to ensure we don't see the latter, lest
					we leave some bytes dangling.
					*/
					if      ( buf[3]==0x00                                 ) return 0;
					else if ( buf[3]==0xe0 && buf[4]==0x00                 ) return 0;
					else if ( buf[3]==0xe0 && buf[4]==0xf0 && buf[5]==0x00 ) return 0;

					if (
						/* buf[0]==0xe0 && buf[1]==0xf0 && buf[2]==0x7c && */
						buf[3]==0xe0 && buf[4]==0xf0 && buf[5]==0x12
					) {
						MOSS_RELEASE( INPDEV_BTN_PrintScreen );
						return 6;
					}

					MOSS_RELEASE( INPDEV_BTN_PrintScreen );
					return 3;
				}
			}
		}

		// Probably IBM key numbers 75--89, 95
		bool got;
		uint8_t const* ptr = buf;
		if      ( ptr[1] == 0x12 ) // Num-Locked 75--89 make code
		{
			ptr += 2;
			got = _parse_set2_helper_make( dev_id, &ptr );
		}
		else if ( ptr[1] == 0xf0 )
		{
			if      ( ptr[2] == 0x59 ) // Right Shift make code
			{
				ptr += 3;
				if ( ptr[0]==0xe0 && ptr[1]==0xf0 && ptr[2]==0x12 ) ptr+=3; // Both shifts
				got = _parse_set2_helper_make( dev_id, &ptr );
			}
			else if ( ptr[2] == 0x12 ) // Left Shift make code
			{
				ptr += 3;
				got = _parse_set2_helper_make( dev_id, &ptr );
			}
			else // Some kind of break code
			{
				got = _parse_set2_helper_break( dev_id, &ptr );
				if ( got && ptr[0]==0xe0 )
				{
					if ( ptr[1]==0xf0 && ptr[2]==0x12 ) ptr+=3; // Num-Locked 75--89 break code
					else
					{
						if ( ptr[1] == 0x12 ) ptr+=2; // Left Shift
						if ( ptr[0]==0xe0 && ptr[1]==0x59 ) ptr+=2; // Right Shift
					}
				}
			}
		}
		else // Make code
		{
			got = _parse_set2_helper_make( dev_id, &ptr );
		}

		return got ? (int)(ptr-buf) : 0;
	}
	else if (
		buf[0]==0xe1 && buf[1]==0x14 && buf[2]==0x77 && buf[3]==0xe1 &&
		buf[4]==0xf0 && buf[5]==0x14 && buf[6]==0xf0 && buf[7]==0x77
	) {
		MOSS_PRESS_AND_RELEASE( INPDEV_BTN_PauseBreak ); // PauseBreak
		return 8;
	}

	return 0;
}

MOSS_ND unsigned inpdev_kb_parse_ps2_sc_set3( DEVID dev_id, uint8_t const buf[2] )
{
	uint8_t const* ptr = buf;

	bool was_release = false;
	if ( *ptr == 0xf0 ) { ++ptr; was_release=true; }

	if ( *ptr<=7 || *ptr>=142 ) return 0;
	INPDEV_BTN inpdev_btn = _sc_set3_table[ *ptr - 7 ];
	if ( inpdev_btn == INV ) return 0;

	if ( was_release )
	{
		MOSS_RELEASE( inpdev_btn );
		return 2;
	}
	else
	{
		MOSS_PRESS  ( inpdev_btn );
		return 1;
	}
}

#undef MOSS_PRESS_AND_RELEASE
#undef MOSS_RELEASE
#undef MOSS_PRESS
