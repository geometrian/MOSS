#include "keyboard.h"

#include "../../io/io.h"
#include "../../kernel.h"

#include "../keys.h"

#include "controller_ps2.h"


namespace MOSS { namespace Input { namespace Devices {


DevicePS2Keyboard::DevicePS2Keyboard(ControllerPS2* controller) : DevicePS2Base(controller) {
	lock_scroll = false;
	lock_num = false;
	lock_caps = false;

	for (int i=0;i<256;++i) {
		keys[i] = false;
	}
}
DevicePS2Keyboard::~DevicePS2Keyboard(void) {}

/*void send_key(uint8_t scancode) {

}*/
void DevicePS2Keyboard::handle_irq(void) /*override*/ {
	//Must read port 0x60 to clear the keyboard interrupt
	//Also, we want the scan code!
	uint8_t scan_code = recv();

	if (scan_code&0x80) {
		scan_code ^= 0x80;
		keys[scan_code] = false;
	} else {
		keys[scan_code] = true;
	}

	switch (scan_code) {
		case Keys::Codes::KEY_LOCK_SCROLL: lock_scroll=!lock_scroll; set_LEDs(); break;
		//TODO: this
		//case Keys::Codes::   KEY_LOCK_NUM: lock_num=!lock_num; set_LEDs(); break;
		case Keys::Codes::  KEY_LOCK_CAPS: lock_caps=!lock_caps; set_LEDs(); break;
		/*case Keys::Codes::KEY_CTRL: break; //TODO: distinction between sides?
		case Keys::Codes::KEY_ALT: break; //TODO: distinction between sides?
		case Keys::Codes::KEY_SHIFT_L: break;
		case Keys::Codes::KEY_SHIFT_R: break;*/
	}

	Keys::Codes::MossKey key = (Keys::Codes::MossKey)(scan_code);
	if (keys[scan_code]) {
		Keys::Event event(key,true);
		Kernel::handle_key_down(event);
	} else {
		Keys::Event event(key,false);
		Kernel::handle_key_up(event);
	}
}

uint8_t DevicePS2Keyboard::get_command_register(void) const /*override*/ {
	return CommandRegister;
}
uint8_t DevicePS2Keyboard::get_inputbuffer(void) const /*override*/ {
	return InputBuffer;
}

bool DevicePS2Keyboard::get_shift_state(void) const {
	return lock_caps ^ keys[Keys::Codes::KEY_SHIFT_L] ^ keys[Keys::Codes::KEY_SHIFT_R];
}

bool DevicePS2Keyboard::set_LEDs(void) {
	//Set the keyboard's LEDs -> 0xFA(ACK)/0xFE(resend)
	//Expects one data byte:
	//	0b00000[caps][num][scroll]
	//	Other bits may be used in international keyboards for other purposes
	uint8_t data = 0;
	if (lock_scroll) data |= 0x01;
	if (   lock_num) data |= 0x02;
	if (  lock_caps) data |= 0x04;
	for (int i=0;i<MOSS_MAX_PS2_ATTEMPTS;++i) {
		send(0xED);
		send(data);
		if (recv()==0xFA) return true;
	}
	return false;
}
bool DevicePS2Keyboard::echo(void) const {
	//Echo command (for diagnostic purposes/device removal detection) -> 0xEE(Echo)/0xFE(resend)
	//Expects no data bytes
	for (int i=0;i<MOSS_MAX_PS2_ATTEMPTS;++i) {
		send(0xEE);
		if (recv()==0xEE) return true;
	}
	return false;
}
int DevicePS2Keyboard::get_scancode(void) const {
	//Retrieves the scancode the keyboard is currently using -> 0xFA(ACK),[scan code set number]/0xFE(resend)
	//Expects one data byte (0x00, the get flag of the 0xF0 command)
	//This method returns the scancode number on success, -1 on failure.
	for (int i=0;i<MOSS_MAX_PS2_ATTEMPTS;++i) {
		send(0xF0);
		if (recv()==0xFA) return recv();
	}
	return -1;
}
bool DevicePS2Keyboard::set_scancode(int scancode) {
	//Sets the keyboard's scancode -> 0xFA(ACK)/0xFE(resend)
	//Expects one data byte (the scancode set, which should be 0x01/0x02/0x03 for scancode sets 1/2/3)
	for (int i=0;i<MOSS_MAX_PS2_ATTEMPTS;++i) {
		send(0xF0);
		send(scancode);
		if (recv()==0xFA) return true;
	}
	return false;
}
int DevicePS2Keyboard::identify(void) {
	//http://wiki.osdev.org/%228042%22_PS/2_Controller#Detecting_PS.2F2_Device_Types
	//Asks the PS/2 device to identify itself -> 0xFA(ACK),[none, 1, or 2 bytes]
	//Expects no data bytes
	//Sets the indentification of the device (0 on success, -1 on failure, -2 when not recognized)

	//TODO: http://wiki.osdev.org/PS/2_Keyboard#Special_Bytes seems to imply that this can only return ACK?
	send(0xF2);
	if (recv()==0xFA) {
		//No response indicates "Ancient AT keyboard with translation enabled in the PS/Controller".  Not supported.
		uint8_t first = recv();
		switch (first) {
			case 0x00:
				//Standard PS/2 mouse
				device_type = DeviceMouse;
				return 0;
			case 0x03:
				//Mouse with scroll wheel
				device_type = DeviceMouseScroll;
				return 0;
			case 0x05:
				//Five-button mouse
				device_type = DeviceMouse5;
				return 0;
			case 0xAB: {
				uint8_t second = recv();
				switch (second) {
					case 0x41:
					case 0xC1:
						//MF2 keyboard with translation enabled in the PS/Controller (not possible for the second PS/2 port)
						device_type = DeviceKeyboardMF2Trans;
						return 0;
					case 0x83:
						//MF2 keyboard
						device_type = DeviceKeyboardMF2;
						return 0;
				}
				//fallthrough
			}
			default:
				return -2;
		}
	}
	return -1;
}
class _TypeMaticByte { public:
	//Bits [0,4] repeat rate (0b00000=30Hz, 0b11111=2Hz) (TODO: How is <= that possible?)
	//Bits [5,6] delay before keys start repeating (0b00=250ms,0b01=500ms,0b10=750ms,0b11=1000ms)?
	//Bit 7 must be 0.
	uint8_t          repeat_rate : 5;
	uint8_t delay_before_repeats : 2;
	bool                  unused : 1;
} __attribute__((packed));
bool DevicePS2Keyboard::set_typematic(int delay_before_typematic_ms, int repeat_rate_hz) {
	//Set autorepeat ("typematic") delay and repeat rate -> 0xFA(ACK)/0xFE(resend)
	//Expects one data byte (see _TypeMaticByte).
	//delay_before_typematic_ms must be one of {250,500,750,1000}.
	//repeat_rate_hz must be in the range [2,30]?
	union {
		_TypeMaticByte data;
		uint8_t byte;
	} convert;
	convert.data.         repeat_rate =            repeat_rate_hz;
	convert.data.delay_before_repeats = delay_before_typematic_ms;
	convert.data.              unused =                         0;
	for (int i=0;i<MOSS_MAX_PS2_ATTEMPTS;++i) {
		send(0xF3);
		send(convert.byte);
		if (recv()==0xFA) return true;
	}
	return false;
}
bool DevicePS2Keyboard::scanning_enable(void) {
	//Enable scanning (keyboard will send scan codes) -> 0xFA(ACK)/0xFE(resend)
	//Expects no data bytes
	for (int i=0;i<MOSS_MAX_PS2_ATTEMPTS;++i) {
		send(0xF4);
		if (recv()==0xFA) return true;
	}
	return false;
}
//TODO: OSDev seems to think that 0xF5 disables scanning, possibly also resetting to default parameters, whereas http://www.brokenthorn.com/Resources/OSDev19.html
//seems to think that 0xF5 resets to power on condition and then waits for the enable command.  I.e., osdev thinks the reset may happen, BrokenThorn avers it will.
bool DevicePS2Keyboard::scanning_disable(void) {
	//Disable scanning (keyboard will not send scan codes) -> 0xFA(ACK)/0xFE(resend)
	//Expects no data bytes
	for (int i=0;i<MOSS_MAX_PS2_ATTEMPTS;++i) {
		send(0xF5);
		if (recv()==0xFA) return true;
	}
	return false;
}
//TODO: OSDev thinks that 0xF6 sets default parameters, but BrokenThorn thinks it also enables scanning.
bool DevicePS2Keyboard::reset(void) {
	//Resets keyboard to defaults -> 0xFA(ACK)/0xFE(resend)
	//Expects no data bytes
	for (int i=0;i<MOSS_MAX_PS2_ATTEMPTS;++i) {
		send(0xF6);
		if (recv()==0xFA) return true;
	}
	return false;
}
bool DevicePS2Keyboard::self_test(void) {
	//Reset keyboard to power on state and start self test -> 0xAA(passed)/0xFC(failed)/0xFD(failed)/0xFE(resend)
	//Expects no data bytes
	for (int i=0;i<MOSS_MAX_PS2_ATTEMPTS;++i) {
		send(0xFF);
		switch (recv()) {
			case 0xAA: return true;
			case 0xFC:
			case 0xFD:
				return false;
			case 0xFE: continue;
		}
	}
	return false;
}


}}}