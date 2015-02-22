#include "interface_keyboard_ps2.h"

#include "../../io/io.h"
#include "../../kernel.h"

#include "../keys.h"

#include "controller_ps2.h"


namespace MOSS { namespace Input { namespace Devices {


InterfaceDevicePS2Keyboard::InterfaceDevicePS2Keyboard(ControllerPS2* controller, int device_index, DeviceType const& device_type) : InterfaceDevicePS2Base(controller,device_index,device_type) {
	lock_scroll = false;
	lock_num = false;
	lock_caps = false;

	for (int i=0;i<256;++i) {
		keys[i] = false;
	}
}
InterfaceDevicePS2Keyboard::~InterfaceDevicePS2Keyboard(void) {}

bool InterfaceDevicePS2Keyboard::handle_irq(void) /*override*/ {
	if (!InterfaceDevicePS2Base::handle_irq()) return false;

	//Must read port 0x60 to clear the keyboard interrupt
	//Also, we want the scan code!
	uint8_t scan_code;
	controller->recv_data(&scan_code);

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
		kernel->handle_key_down(Keys::Event(key,true));
	} else {
		kernel->handle_key_up(Keys::Event(key,false));
	}

	return true;
}

bool InterfaceDevicePS2Keyboard::get_shift_state(void) const {
	return lock_caps ^ keys[Keys::Codes::KEY_SHIFT_L] ^ keys[Keys::Codes::KEY_SHIFT_R];
}

bool InterfaceDevicePS2Keyboard::set_LEDs(void) {
	//Set the keyboard's LEDs -> 0xFA(ACK)/0xFE(resend)
	//Expects one data byte:
	//	0b00000[caps][num][scroll]
	//	Other bits may be used in international keyboards for other purposes
	uint8_t data = 0;
	if (lock_scroll) data |= 0x01;
	if (   lock_num) data |= 0x02;
	if (  lock_caps) data |= 0x04;
	send_command_device(0xED);
	controller->send_data(data);
	wait_response();
	return true;
}
#if 0
bool InterfaceDevicePS2Keyboard::echo(void) const {
	//Echo command (for diagnostic purposes/device removal detection) -> 0xEE(Echo)/0xFE(resend)
	//Expects no data bytes
	for (int i=0;i<MOSS_MAX_PS2_ATTEMPTS;++i) {
		send(0xEE);
		if (recv()==0xEE) return true;
	}
	return false;
}
#endif
int InterfaceDevicePS2Keyboard::get_scancode(void) {
	//Retrieves the scancode the keyboard is currently using -> 0xFA(ACK),[scan code set number]/0xFE(resend)
	//Expects one data byte (0x00, the get flag of the 0xF0 command)
	//This method returns the scancode number on success, -1 on failure.
	send_command_device(0xF0);
	//wait_response();
	assert_term(false,"Not implemented!");
	return -1;
}
void InterfaceDevicePS2Keyboard::set_scancode(int scancode) {
	//Sets the keyboard's scancode -> 0xFA(ACK)/0xFE(resend)
	//Expects one data byte (the scancode set, which should be 0x01/0x02/0x03 for scancode sets 1/2/3)
	send_command_device(    0xF0);
	wait_response();
	send_command_device(scancode);
	wait_response();
}
#if 0
class _TypeMaticByte { public:
	//Bits [0,4] repeat rate (0b00000=30Hz, 0b11111=2Hz) (TODO: How is <= that possible?) <= This is how: http://www.computer-engineering.org/ps2keyboard/
	//Bits [5,6] delay before keys start repeating (0b00=250ms,0b01=500ms,0b10=750ms,0b11=1000ms)?
	//Bit 7 must be 0.
	uint8_t          repeat_rate : 5;
	uint8_t delay_before_repeats : 2;
	bool                  unused : 1;
} __attribute__((packed));
bool InterfaceDevicePS2Keyboard::set_typematic(int delay_before_typematic_ms, int repeat_rate_hz) {
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
bool InterfaceDevicePS2Keyboard::scanning_enable(void) {
	//Enable scanning (keyboard will send scan codes) -> 0xFA(ACK)/0xFE(resend)
	//Expects no data bytes
	for (int i=0;i<MOSS_MAX_PS2_ATTEMPTS;++i) {
		send(0xF4);
		if (recv()==0xFA) return true;
	}
	return false;
}

bool InterfaceDevicePS2Keyboard::scanning_disable(void) {
	//Disable scanning (keyboard will not send scan codes) -> 0xFA(ACK)/0xFE(resend)
	//Expects no data bytes
	for (int i=0;i<MOSS_MAX_PS2_ATTEMPTS;++i) {
		send(0xF5);
		if (recv()==0xFA) return true;
	}
	return false;
}
//TODO: OSDev thinks that 0xF6 sets default parameters, but BrokenThorn thinks it also enables scanning.
bool InterfaceDevicePS2Keyboard::reset(void) {
	//Resets keyboard to defaults -> 0xFA(ACK)/0xFE(resend)
	//Expects no data bytes
	for (int i=0;i<MOSS_MAX_PS2_ATTEMPTS;++i) {
		send(0xF6);
		if (recv()==0xFA) return true;
	}
	return false;
}
bool InterfaceDevicePS2Keyboard::self_test(void) {
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
#endif


}}}