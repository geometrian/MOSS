#pragma once
#include "../../includes.h"

#include "device.h"


namespace MOSS {


namespace Input {


//Keys are listed in the order they appear on my keyboard.
//MACRO format:
//	--Key name stub, key name stub (shifted)
//	--ascii, ascii shifted
//	--XT scan code make, XT scan code break, both should be treated as uint64_t, with
//	  bytes progressing from LSB to MSB.  A value of 0x00 means that there is no code.
//See:
//	http://msdn.microsoft.com/en-us/library/ms894073.aspx
//	http://www.brokenthorn.com/Resources/OSDevScanCodes.html
//	http://wiki.osdev.org/PS2_Keyboard
//TODO: this!
#define KEY(MACRO)\
	MACRO(  ESCAPE,INVALID,   "ESCAPE","INVALID", 0x01,0x81)\
	MACRO(      F1,INVALID,       "F1","INVALID", 0x3B,0xBB)\
	MACRO(      F2,INVALID,       "F2","INVALID", 0x3C,0xBC)\
	MACRO(      F3,INVALID,       "F3","INVALID", 0x3D,0xBD)\
	MACRO(      F4,INVALID,       "F4","INVALID", 0x3E,0xBE)\
	MACRO(      F5,INVALID,       "F5","INVALID", 0x3F,0xBF)\
	MACRO(      F6,INVALID,       "F6","INVALID", 0x40,0xC0)\
	MACRO(      F7,INVALID,       "F7","INVALID", 0x41,0xC1)\
	MACRO(      F8,INVALID,       "F8","INVALID", 0x42,0xC2)\
	MACRO(      F9,INVALID,       "F9","INVALID", 0x43,0xC3)\
	MACRO(     F10,INVALID,      "F10","INVALID", 0x44,0xC4)\
	MACRO(     F11,INVALID,      "F11","INVALID", 0x57,0xD7)\
	MACRO(     F12,INVALID,      "F12","INVALID", 0x58,0xD8)\
	/*Print screen/SysRq?*/\
	MACRO(   PAUSE,  BREAK,    "PAUSE",  "BREAK", 0x0000C59DE1451DE1,0x00)\
	MACRO(  INSERT,INVALID,   "INSERT","INVALID", 0x52E0,0xD2E0)\
	MACRO(  DELETE,INVALID,   "DELETE","INVALID", 0x53E0,0xD3E0)\
	MACRO(KP_SLASH,INVALID, "KP SLASH","INVALID", 0x35E0,0xB5E0)\
	MACRO(KP_TIMES,INVALID, "KP TIMES","INVALID",   0x37,  0xB7)\
	\
	MACRO(TICK,TILDE "`","~", 0x29,0x89)\
	MACRO(1, "1", 0x02,0x82)\
	MACRO(2, "2", 0x03,0x83)\
	MACRO(3, "3", 0x04,0x84)\
	MACRO(4, "4", 0x05,0x85)\
	MACRO(5, "5", 0x06,0x86)\
	MACRO(6, "6", 0x07,0x87)\
	MACRO(7, "7", 0x08,0x88)\
	MACRO(8, "8", 0x09,0x89)\
	MACRO(9, "9", 0x0A,0x8A)\
	MACRO(0, "0", 0x0B,0x8B)\
	MACRO(MINUS, "-", 0x0C,0x8C)\
	MACRO(EQUALS, "=", 0x0D,0x8D)\
	MACRO(BKSP, "BKSP", 0x0E,0x8E)\
	MACRO(KP_DOT, "KP .", 0x53,0xD3)\
	MACRO(KP_PLUS, "KP +", 0x4E,0xCE)\
	MACRO(KP_MINUS, "KP -", 0x4A,0xCA)\
	\
	MACRO(TAB, "TAB", 0x0F,0x8F)\
	MACRO(q,Q, "q","Q", 0x10,0x90)\
	MACRO(w,W, "w","W", 0x11,0x91)\
	MACRO(e,E, "e","E", 0x12,0x92)\
	MACRO(r,R, "r","R", 0x13,0x93)\
	MACRO(t,T, "t","T", 0x14,0x94)\
	MACRO(y,Y, "y","Y", 0x15,0x95)\
	MACRO(u,U, "u","U", 0x16,0x96)\
	MACRO(i,I, "i","I", 0x17,0x97)\
	MACRO(o,O, "o","O", 0x18,0x98)\
	MACRO(p,P, "p","P", 0x19,0x99)\
	MACRO(BRACKET_L,BRACE_L, "[","{", 0x1A,0x9A)\
	MACRO(BRACKET_R,BRACE_R, "]","}", 0x1B,0x9B)\
	MACRO(BACKSLASH,PIPE, "\\","|" 0x2B,0xAB)\
	MACRO(KP_7, "KP 7", 0x47,0xC7)\
	MACRO(KP_8, "KP 8", 0x48,0xC8)\
	MACRO(KP_9, "KP 9", 0x49,0xC9)\
	\
	MACRO(CAPS, "CAPS", 0x3A,0xBA)\
	MACRO(a,A, "a","A", 0x1E,0x9E)\
	MACRO(s,S, "s","S", 0x1F,0x9F)\
	MACRO(d,D, "d","D", 0x20,0xA0)\
	MACRO(f,F, "f","F", 0x21,0xA1)\
	MACRO(g,G, "g","G", 0x22,0xA2)\
	MACRO(h,H, "h","H", 0x23,0xA3)\
	MACRO(j,J, "j","J", 0x24,0xA4)\
	MACRO(k,K, "k","K", 0x25,0xA5)\
	MACRO(l,L, "l","L", 0x26,0xA6)\
	MACRO(SEMICOLON,COLON, ";",":", 0x27,0xA7)\
	MACRO(QUOTE,DOUBLEQUOTE, "\'","\"", 0x28,0xA8)\
	MACRO(ENTER, "ENTER", 0x1C,0x9C)\
	MACRO(KP_4, "KP 4", 0x4B,0xCB)\
	MACRO(KP_5, "KP 5", 0x4C,0xCC)\
	MACRO(KP_6, "KP 6", 0x4D,0xCD)\
	\
	MACRO(SHIFT_L, "L SHFT", 0x2A,0xAA)\
	MACRO(z,Z, "z","Z", 0x2C,0xAC)\
	MACRO(x,X, "x","X", 0x2D,0xAD)\
	MACRO(c,C, "c","C", 0x2E,0xAE)\
	MACRO(v,V, "v","V", 0x2F,0xAF)\
	MACRO(b,B, "b","B", 0x30,0xB0)\
	MACRO(n,N, "n","N", 0x31,0xB1)\
	MACRO(m,M, "m","M", 0x32,0xB2)\
	MACRO(COMMA, ",", 0x33,0xB3)\
	MACRO(DOT, ".", 0x34,0xB4)\
	MACRO(SLASH, "/", 0x35,0xB5)\
	MACRO(SHIFT_R, "R SHFT", 0x36,0xB6)\
	MACRO(ARROW_U, "U ARROW", 0xE0,48,0xE0,C8)\
	MACRO(KP_1, "KP 1", 0x4F,0xCF)\
	MACRO(KP_2, "KP 2", 0x50,0xD0)\
	MACRO(KP_3, "KP 3", 0x51,0xD1)\
	\
	MACRO(CTRL_L, "L CTRL", 0x1D,0x9D)\
	/*Function?*/\
	MACRO(GUI_L, "L GUI", 0xE0,5B,0xE0,DB) /*?*/\
	MACRO(ALT_L, "L ALT", 0x38,0xB8)\
	MACRO(SPACE, "SPACE", 0x39,0xB9)\
	MACRO(ALT_R, "R ALT", 0xE0,38,0xE0,B8)\
	MACRO(GUI_R, "R GUI", 0xE0,5C,0xE0,DC)\
	MACRO(CTRL_R, "R CTRL", 0xE0,1D,0xE0,9D)\
	MACRO(ARROW_L, "L ARROW", 0xE0,4B,0xE0,CB)\
	MACRO(ARROW_D, "D ARROW", 0xE0,50,0xE0,D0)\
	MACRO(ARROW_R, "R ARROW", 0xE0,4D,0xE0,CD)\
	MACRO(KP_0, "KP 0", 0x52,0xD2)\
	MACRO(KP_ENTER, "KP EN", 0xE0,1C,0xE0,9C)\
	\
	MACRO(SCROLL, "SCROLL", 0x46,0xC6)\
	\
	MACRO(NUM, "NUM", 0x45,0xC5)\
	MACRO(PG DN, "PG DN", 0xE0,51,0xE0,D1)\
	MACRO(PG UP, "PG UP", 0xE0,49,0xE0,C9)\
	MACRO(PRNT, "PRNT", 0xSCRN,0xE0,2A,)\
	MACRO(APPS, "APPS", 0xE0,5D,0xE0,DD)\
	MACRO(E0,37, "E0,37", 0xE0,B7,,0xE0,AA)\
	\
	MACRO(END, "END", 0xE0,4F,0xE0,CF)\
	MACRO(HOME, "HOME", 0xE0,47,0xE0,97)\


class KeyEvent { public:
	const enum Key {
	} key;
	const bool pressed;
	KeyEvent(Key key, bool pressed) : key(key), pressed(pressed) {}
};


} namespace Devices {


//http://www.brokenthorn.com/Resources/OSDev19.html

//Note: the motherboard emulates USB keyboards and USB mice as PS/2 devices.  Therefore, this code should
//work for them as well.  However, http://wiki.osdev.org/%228042%22_PS/2_Controller#USB_Legacy_Support
//says that you should disable legacy support.  TODO: that.

//There is a keyboard encoder and a keyboard controller.  The encoder lives on the keyboard itself
//and stores data about it.  The keyboard controller lives on the motherboard and communicates
//with the encoder, producing IRQs.

//Key codes are either make and break.  Make is the press, break is the release.

//Note: at the hardware level, const-correctness is pretty-much meaningless.  "const" is added here
//when the method oughtn't to semantically change the state of something.  I.e., even though the
//data buffers of the PS/2 device or controller change, "const" implies their state should be
//preserved.

class DevicePS2Keyboard : public DevicePS2Base {
	private:
		enum DeviceIO {
			InputBuffer     = 0x60,
			CommandRegister = 0x60
		};

		bool lock_scroll;
		bool lock_num;
		bool lock_caps;

	public:
		DevicePS2Keyboard(ControllerPS2* controller);
		virtual ~DevicePS2Keyboard(void);

		void handle_irq(void) override;

		uint8_t get_command_register(void) const override;
		uint8_t get_inputbuffer(void) const override;

		//http://wiki.osdev.org/PS/2_Keyboard#Commands
		bool set_LEDs(void);
		bool echo(void) const;
		int get_scancode(void) const;
		bool set_scancode(int scancode);
		//TODO: move to DevicePS2Base and call on construction
		int identify(void);
		bool set_typematic(int delay_before_typematic_ms, int repeat_rate_hz);
		bool scanning_enable(void);
		//TODO: OSDev seems to think that 0xF5 disables scanning, possibly also resetting to default parameters, whereas http://www.brokenthorn.com/Resources/OSDev19.html
		//seems to think that 0xF5 resets to power on condition and then waits for the enable command.  I.e., OSDev thinks the reset may happen, BrokenThorn avers it will.
		bool scanning_disable(void);
		//TODO: OSDev thinks that 0xF6 sets default parameters, but BrokenThorn thinks it also enables scanning.
		bool reset(void);
		//There are a lot of other commands that aren't implemented here:
		//0xF7 //Set all keys to autorepeat (PS/2, scancode 3 only)
		//0xF8 //Set all keys to send make code and break code (PS/2, scancode 3 only)
		//0xF9 //Set all keys to generate only make codes (scancode 3 only)
		//0xFA //Set all keys to autorepeat and generate make/break codes (scancode 3 only)
		//0xFB //Set a single key to autorepeat (scancode 3 only)
		//0xFC //Set a single key to generate make and break codes (scancode 3 only)
		//0xFD //Set a single key to generate only break codes (scancode 3 only)
		//0xFE //Resend last result
		bool self_test(void);
};


//okay &= keyboard->read_buffer()==0x00;
//0x00 = Success; no errors
//0x01 = Keyboard clock line stuck low
//0x02 = Keyboard clock line stuck high
//0x03 = Keyboard data line stuck high
//0xFF = General error

//TODO: this
/*// scan error codes ------------------------------------------
enum KYBRD_ERROR {

	KYBRD_ERR_BUF_OVERRUN			=	0x00,
	KYBRD_ERR_ID_RET				=	0x83AB,
	KYBRD_ERR_BAT					=	0xAA, //note: can also be L. shift key make code
	KYBRD_ERR_ECHO_RET				=	0xEE,
	KYBRD_ERR_ACK					=	0xFA,
	KYBRD_ERR_BAT_FAILED			=	0xFC,
	KYBRD_ERR_DIAG_FAILED			=	0xFD,
	KYBRD_ERR_RESEND_CMD			=	0xFE,
	KYBRD_ERR_KEY					=	0xFF
};*/


}}