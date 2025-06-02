#pragma once

//#include "stdafx.h"

#include "inpdev_defs.autogen.h"



struct InpDevs_Controller;



/*
Input Device: Keyboard
*/

struct InpDev_Keyboard
{
	DEVTYPE type;

	bool is_ansi_like;

	uint32_t _key_states[ 256 / 32 ];

	// Varies by the keyboard's current scancode set.
	unsigned(*inpdev_kb_parse_ps2_sc_seti)( DEVID dev_id, uint8_t const buf[] );
};



/*
Input Device: Mouse
*/

struct InpDev_Mouse
{
	DEVTYPE type;

	vec2i pos;
	vec2i rel;

	int rel_scroll;

	bool buttons[5]; // left, middle, right, other A, other B
};



/*
Input Devices Controller
*/

struct InpDev_Base
{
	union
	{
		DEVTYPE type;

		uint8_t _raw[
			MAX( sizeof(struct InpDev_Keyboard), sizeof(struct InpDev_Mouse) )
		];
	};
};

struct InpDevs_Controller
{
	struct InpDev_Base devices[2];
	uint32_t           devices_count;
};
extern struct InpDevs_Controller g_inpdev_ctrl;

void inpdevs_init(void);

MOSS_ND DEVID inpdevs_add_kb   (void);
MOSS_ND DEVID inpdevs_add_mouse(void);

void inpdevs_got_btn_press            ( DEVID dev_id, INPDEV_BTN inpdev_btn );
void inpdevs_got_btn_release          ( DEVID dev_id, INPDEV_BTN inpdev_btn );
void inpdevs_got_btn_press_and_release( DEVID dev_id, INPDEV_BTN inpdev_btn );



MOSS_ND_INLINE struct InpDev_Keyboard* inpdev_kb_get( DEVID dev_id )
{
	__moss_assert( dev_id<g_inpdev_ctrl.devices_count, "Invalid device %u!", dev_id );
	__moss_assert( g_inpdev_ctrl.devices[dev_id].type==DEVTYPE_InputKeyboard,
		"Invalid device type %d!", g_inpdev_ctrl.devices[dev_id].type
	);
	return (struct InpDev_Keyboard*)( g_inpdev_ctrl.devices + dev_id );
}

MOSS_ND unsigned inpdev_kb_parse_ps2_sc_set1( DEVID dev_id, uint8_t const buf[6] );
MOSS_ND unsigned inpdev_kb_parse_ps2_sc_set2( DEVID dev_id, uint8_t const buf[8] );
MOSS_ND unsigned inpdev_kb_parse_ps2_sc_set3( DEVID dev_id, uint8_t const buf[2] );

MOSS_ND_INLINE bool inpdev_kb_get_pressed ( DEVID dev_id, INPDEV_BTN inpdev_btn )
{
	struct InpDev_Keyboard const* kb = inpdev_kb_get( dev_id );
	return (bool)( kb->_key_states[inpdev_btn/32] & (1u<<inpdev_btn%32) );
}
MOSS_INLINE    void inpdev_kb_set_pressed ( DEVID dev_id, INPDEV_BTN inpdev_btn )
{
	struct InpDev_Keyboard* kb = inpdev_kb_get( dev_id );

	uint32_t block = kb->_key_states[ inpdev_btn / 32 ];
	block |= 1u << inpdev_btn%32;
	kb->_key_states[ inpdev_btn / 32 ] = block;
}
MOSS_INLINE    void inpdev_kb_set_released( DEVID dev_id, INPDEV_BTN inpdev_btn )
{
	struct InpDev_Keyboard* kb = inpdev_kb_get( dev_id );

	uint32_t block = kb->_key_states[ inpdev_btn / 32 ];
	block &= ~(1u << inpdev_btn%32);
	kb->_key_states[ inpdev_btn / 32 ] = block;
}



MOSS_ND_INLINE struct InpDev_Mouse* inpdev_mouse_get( DEVID dev_id )
{
	__moss_assert( dev_id<g_inpdev_ctrl.devices_count, "Invalid device %u!", dev_id );
	__moss_assert( g_inpdev_ctrl.devices[dev_id].type==DEVTYPE_InputMouse,
		"Invalid device type %d!", g_inpdev_ctrl.devices[dev_id].type
	);
	return (struct InpDev_Mouse*)( g_inpdev_ctrl.devices + dev_id );
}

void inpdev_mouse_moved( DEVID dev_id, vec2i rel );
void inpdev_mouse_scroll( DEVID dev_id, int rel_scroll );











#if 0

union InpDev_EventBtn
{
	struct
	{
		struct
		{
			uint8_t byte_code   : 6;

			// Note can be *both* a press and a release (e.g. for the Pause / Break key in PS/2 scancode
			// sets 1 and 2.)
			uint8_t was_press   : 1;
			uint8_t was_release : 1;
		};

		HIDBTN hid_btn;

		uint8_t payload[2];
	};
	uint32_t packed;
	uint8_t bytes[4];
};
static_assert( sizeof(union InpDev_EventBtn) == 2 );





void inpdev_evt_btn( union InpDev_EventBtn evt )
{
	switch ( evt.byte_code )
	{
		case HIDCMD_KeyPress_And_EmitPayloadAsUTF16:
			inpdev_kb_set_pressed( &hid_ctrl.kb, evt.hid_btn, true );
			kernel_got_keypress( evt.hid_btn );
			break;
		case HIDCMD_KeyPress_And_SavePayloadAsUTF16:
			inpdev_kb_set_pressed( &hid_ctrl.kb, evt.hid_btn, true );
			kernel_got_keypress( evt.hid_btn );
			break;

		case HIDCMD_KeyPress_And_KeyLockToggle:
			inpdev_kb_set_toggle( &hid_ctrl.kb, evt.hid_btn );
			kernel_got_keypress( evt.hid_btn );
			break;

		case HIDCMD_KeyPress_And_KeyLockSet:
			inpdev_kb_set_toggle( &hid_ctrl.kb, evt.hid_btn );
			kernel_got_keypress( evt.hid_btn );
			break;

		case HIDCMD_KeyPress_And_KeyLockClear:
			break;


		case HIDCMD_KeyRelease:
			kernel_got_keyrelease( evt.hid_btn );
			break;

		default:
			break;
	}
}








/*
# Great resources here: https://kbdlayout.info/
# How Linux handles it: https://man7.org/linux/man-pages/man5/keymaps.5.html


#def key( **kwargs ):
#	pass
#
#def ctrl_key( name ):
#	pass
#
#def basic_key(  ):
#	pass	
#
#us_qwerty = {
#	65 : [ 'a', 'A' ]
#}*/





/*
Keyboard Input

The keyboard converts its input into a `KEYNUM`, a 1-byte number roughly corresponding to the
physical location of the key.  Then, it is translated into an abstract key according to the user's
keyboard layout.

This is somewhat involved, so see "doc/moss_keyboard.md" for a full explanation.
*/





// An entry in a key map, stores the control character or associated unicode character.
struct KeyInfo
{
	// UTF-32 (i.e., the Unicode codepoint) of the associated character, if applicable.  The largest
	// UTF-32 codepoint is U+10FFFF (i.e. plane 17 number 65535, or 17∙2¹⁶-1).  This needs 21 bits.
	uint32_t utf32; //: 21;
};
static_assert( sizeof(struct KeyInfo) == 4 );

struct KeyMap
{
	struct KeyInfo keys[256];
};

extern struct KeyMap keymap_us_qwerty;

struct KeyMap keymap_us_qwerty =
{ .keys = {



} };





union ModifierKeys
{
	struct
	{
		uint16_t left_ctrl    : 1;
		uint16_t right_ctrl   : 1;

		uint16_t left_alt     : 1;
		uint16_t alt_gr       : 1;

		uint16_t left_shift   : 1;
		uint16_t right_shift  : 1;

		uint16_t left_os_key  : 1;
		uint16_t right_os_key : 1;

		uint16_t fn           : 1;

		uint16_t caps_lock    : 1;
		uint16_t num_lock     : 1;
		uint16_t scroll_lock  : 1;

		uint16_t insert       : 1;
		uint16_t mute         : 1;

		uint16_t              : 2;
	};
	struct
	{
		uint16_t ctrls   : 2;
		uint16_t alts    : 2;
		uint16_t shifts  : 2;
		uint16_t os_keys : 2;
		uint16_t         : 1;
		uint16_t locks   : 3;
		uint16_t         : 2;
		uint16_t         : 2;
	};
	uint16_t packed;
};

MOSS_ND char const* key_get_name( KEYNUM key_num );






typedef enum
{
	EVENT_MOUSE_MOTION

	EVENT_KEY_PRESS,
	EVENT_KEY_RELEASE,
	EVENT_
}
EVENT;

struct EventMouse
{
	EVENT type;
};









MOSS_ND union ModifierKeys dev_kb_get_mods( struct DeviceKeyboard const* kb )
{
	// Everything in first `struct`
	uint16_t packed = (uint16_t)kb->_key_states[0] & 0x0b0F;

	// Everything in second `struct`
	#if 0
	packed |= ror16( (uint16_t)kb->_key_states[1]&0x2001, 9 );
	#else
	packed |= ror16( (uint16_t)kb->_key_states[1], 9 ) & 0x0090;
	#endif

	// Everything in third `struct`
	packed |= (uint16_t)( kb->_key_states[4] >> 13 ) & 0x0010;
	packed |= (uint16_t)( kb->_key_states[5] >> 19 ) & 0x0020;
	packed |= ((uint16_t)kb->_key_states[2]&0x0001) >> 10;

	union ModifierKeys ret;
	ret.packed = packed;
	return ret;
}

#endif
