#pragma once

#include "../../includes.h"


namespace MOSS { namespace Input { namespace Keys { namespace Codes {


//Keys are listed in the order they appear on my keyboard.
//MACRO format:
//	--Key name stub, key name stub (shifted)
//	--ascii, ascii shifted
//	--XT scan code make (break can be found by |-ing 0x80.
//See:
//	http://msdn.microsoft.com/en-us/library/ms894073.aspx
//	http://www.brokenthorn.com/Resources/OSDevScanCodes.html
//	http://wiki.osdev.org/PS2_Keyboard
//TODO: doesn't handle num lock correctly!  See also key2.py, which ignores some because of that!
#define MOSS_KEY_MACRO(MACRO)\
	/*E.g. MACRO(NAME,SHIFT_NAME, DESC,SHIFT_DESC, SC)*/\
	MACRO(     ESCAPE,   INVALID,      "ESCAPE","INVALID", 0x01)\
	MACRO(          1,         1,           "1",      "!", 0x02)\
	MACRO(          2,         2,           "2",      "@", 0x03)\
	MACRO(          3,         3,           "3",      "#", 0x04)\
	MACRO(          4,         4,           "4",      "$", 0x05)\
	MACRO(          5,         5,           "5",      "%", 0x06)\
	MACRO(          6,         6,           "6",      "^", 0x07)\
	MACRO(          7,         7,           "7",      "&", 0x08)\
	MACRO(          8,         8,           "8",      "*", 0x09)\
	MACRO(          9,         9,           "9",      "(", 0x0a)\
	MACRO(          0,         0,           "0",      ")", 0x0b)\
	MACRO(  OEM_MINUS, OEM_MINUS,           "-",      "_", 0x0c)\
	MACRO(   OEM_PLUS,  OEM_PLUS,           "=",      "+", 0x0d)\
	MACRO(  BACKSPACE,   INVALID,   "BACKSPACE","INVALID", 0x0e)\
	MACRO(        TAB,   INVALID,          "\t","INVALID", 0x0f)\
	MACRO(          Q,         Q,           "q",      "Q", 0x10)\
	MACRO(          W,         W,           "w",      "W", 0x11)\
	MACRO(          E,         E,           "e",      "E", 0x12)\
	MACRO(          R,         R,           "r",      "R", 0x13)\
	MACRO(          T,         T,           "t",      "T", 0x14)\
	MACRO(          Y,         Y,           "y",      "Y", 0x15)\
	MACRO(          U,         U,           "u",      "U", 0x16)\
	MACRO(          I,         I,           "i",      "I", 0x17)\
	MACRO(          O,         O,           "o",      "O", 0x18)\
	MACRO(          P,         P,           "p",      "P", 0x19)\
	MACRO(      OEM_4,     OEM_4,           "[",      "{", 0x1a)\
	MACRO(      OEM_6,     OEM_6,           "]",      "}", 0x1b)\
	MACRO(      ENTER,   INVALID,          "\n","INVALID", 0x1c)\
	MACRO(       CTRL,   INVALID,        "CTRL","INVALID", 0x1d)\
	MACRO(          A,         A,           "a",      "A", 0x1e)\
	MACRO(          S,         S,           "s",      "S", 0x1f)\
	MACRO(          D,         D,           "d",      "D", 0x20)\
	MACRO(          F,         F,           "f",      "F", 0x21)\
	MACRO(          G,         G,           "g",      "G", 0x22)\
	MACRO(          H,         H,           "h",      "H", 0x23)\
	MACRO(          J,         J,           "j",      "J", 0x24)\
	MACRO(          K,         K,           "k",      "K", 0x25)\
	MACRO(          L,         L,           "l",      "L", 0x26)\
	MACRO(      OEM_1,     OEM_1,           ";",      ":", 0x27)\
	MACRO(      OEM_7,     OEM_7,           "'",     "\"", 0x28)\
	MACRO(      OEM_3,     OEM_3,           "`",      "~", 0x29)\
	MACRO(    SHIFT_L,   INVALID,       "SHIFT","INVALID", 0x2a)\
	MACRO(      OEM_5,     OEM_5,          "\\",      "|", 0x2b)\
	MACRO(          Z,         Z,           "z",      "Z", 0x2c)\
	MACRO(          X,         X,           "x",      "X", 0x2d)\
	MACRO(          C,         C,           "c",      "C", 0x2e)\
	MACRO(          V,         V,           "v",      "V", 0x2f)\
	MACRO(          B,         B,           "b",      "B", 0x30)\
	MACRO(          N,         N,           "n",      "N", 0x31)\
	MACRO(          M,         M,           "m",      "M", 0x32)\
	MACRO(  OEM_COMMA, OEM_COMMA,           ",",      "<", 0x33)\
	MACRO( OEM_PERIOD,OEM_PERIOD,           ".",      ">", 0x34)\
	MACRO(      OEM_2,     OEM_2,           "/",      "?", 0x35)\
	MACRO(    SHIFT_R,   INVALID, "Right SHIFT","INVALID", 0x36)\
	MACRO(   KP_TIMES,   INVALID,       "NUM *","INVALID", 0x37)\
	MACRO(        ALT,   INVALID,         "ALT","INVALID", 0x38)\
	MACRO(      SPACE,   INVALID,           " ","INVALID", 0x39)\
	MACRO(  LOCK_CAPS,   INVALID,   "CAPS LOCK","INVALID", 0x3a)\
	MACRO(         F1,   INVALID,          "F1","INVALID", 0x3b)\
	MACRO(         F2,   INVALID,          "F2","INVALID", 0x3c)\
	MACRO(         F3,   INVALID,          "F3","INVALID", 0x3d)\
	MACRO(         F4,   INVALID,          "F4","INVALID", 0x3e)\
	MACRO(         F5,   INVALID,          "F5","INVALID", 0x3f)\
	MACRO(         F6,   INVALID,          "F6","INVALID", 0x40)\
	MACRO(         F7,   INVALID,          "F7","INVALID", 0x41)\
	MACRO(         F8,   INVALID,          "F8","INVALID", 0x42)\
	MACRO(         F9,   INVALID,          "F9","INVALID", 0x43)\
	MACRO(        F10,   INVALID,         "F10","INVALID", 0x44)\
	MACRO(      PAUSE,   INVALID,       "PAUSE","INVALID", 0x45)\
	MACRO(LOCK_SCROLL,   INVALID, "SCROLL LOCK","INVALID", 0x46)\
	MACRO(       KP_7,   INVALID,       "Num 7","INVALID", 0x47)\
	MACRO(       KP_8,   INVALID,       "Num 8","INVALID", 0x48)\
	MACRO(       KP_9,   INVALID,       "Num 9","INVALID", 0x49)\
	MACRO(   KP_MINUS,   INVALID,       "Num -","INVALID", 0x4a)\
	MACRO(       KP_4,   INVALID,       "Num 4","INVALID", 0x4b)\
	MACRO(       KP_5,   INVALID,       "Num 5","INVALID", 0x4c)\
	MACRO(       KP_6,   INVALID,       "Num 6","INVALID", 0x4d)\
	MACRO(    KP_PLUS,   INVALID,       "Num +","INVALID", 0x4e)\
	MACRO(       KP_1,   INVALID,       "Num 1","INVALID", 0x4f)\
	MACRO(       KP_2,   INVALID,       "Num 2","INVALID", 0x50)\
	MACRO(       KP_3,   INVALID,       "Num 3","INVALID", 0x51)\
	MACRO(       KP_0,   INVALID,       "Num 0","INVALID", 0x52)\
	MACRO(     KP_DEL,   INVALID,     "Num Del","INVALID", 0x53)\
	MACRO(    SYS_REQ,   INVALID,     "Sys Req","INVALID", 0x54)\
	MACRO(    OEM_102,   OEM_102,          "\\",      "|", 0x56)\
	MACRO(        F11,   INVALID,         "F11","INVALID", 0x57)\
	MACRO(        F12,   INVALID,         "F12","INVALID", 0x58)\
	MACRO(        F13,   INVALID,         "F13","INVALID", 0x7c)\
	MACRO(        F14,   INVALID,         "F14","INVALID", 0x7d)\
	MACRO(        F15,   INVALID,         "F15","INVALID", 0x7e)\
	MACRO(        F16,   INVALID,         "F16","INVALID", 0x7f)
enum MossKey {
	#define MOSS_KEYS(NAME,SHIFT_NAME, DESC,SHIFT_DESC, SC) KEY_##NAME = SC,
	MOSS_KEY_MACRO(MOSS_KEYS)
	#undef MOSS_KEYS
	KEY_INVALID = 0xFF
};


}


class Event { public:
	const Codes::MossKey key;
	const bool pressed;
	Event(Codes::MossKey key, bool pressed) : key(key), pressed(pressed) {}
};

bool is_printable(Codes::MossKey key);

//If the key itself is not "is_printable(...)", then it will return the name of the key.
char const* get_printable(Codes::MossKey key);


}}}
