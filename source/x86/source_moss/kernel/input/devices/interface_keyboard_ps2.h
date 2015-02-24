#pragma once

#include "../../../includes.h"

#include "interface_device_ps2.h"


namespace MOSS { namespace Input { namespace Devices {


//http://www.brokenthorn.com/Resources/OSDev19.html

//Note: the motherboard emulates USB keyboards and USB mice as PS/2 devices.  Therefore, this code should
//	work for them as well.  However, http://wiki.osdev.org/%228042%22_PS/2_Controller#USB_Legacy_Support
//	says that you should disable legacy support.  TODO: that.

//There is a keyboard encoder and a keyboard controller.  The encoder lives on the keyboard itself
//	and stores data about it.  The keyboard controller lives on the motherboard and communicates
//	with the encoder, producing IRQs.

//Key codes are either make and break.  Make is the press, break is the release.

//Note: at the hardware level, const-correctness is pretty-much meaningless.  "const" is added here
//	when the method oughtn't to semantically change the state of something.  I.e., even though the
//	data buffers of the PS/2 device or controller change, "const" implies their state should be
//	preserved.

class ControllerPS2;

class InterfaceDevicePS2Keyboard final : public InterfaceDevicePS2Base {
	private:
		bool lock_scroll;
		bool lock_num;
		bool lock_caps;

		bool keys[256];

	public:
		InterfaceDevicePS2Keyboard(ControllerPS2* controller, int device_index, DeviceType const& device_type);
		virtual ~InterfaceDevicePS2Keyboard(void);

		bool handle_irq(void) override;

		bool get_shift_state(void) const;

		//http://wiki.osdev.org/PS/2_Keyboard#Commands
		bool set_LEDs(void);
		//bool echo(void) const;
		int get_scancode(void);
		void set_scancode(int scancode);
		/*
		bool set_typematic(int delay_before_typematic_ms, int repeat_rate_hz);
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
		bool self_test(void);*/
};

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


}}}
