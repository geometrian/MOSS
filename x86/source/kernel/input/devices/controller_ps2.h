#pragma once
#include "../../../includes.h"

#define MOSS_MAX_PS2_ATTEMPTS 3


namespace MOSS { namespace Input { namespace Devices {


class DevicePS2Keyboard;
class DevicePS2Mouse;

//http://wiki.osdev.org/%228042%22_PS/2_Controller
class ControllerPS2 {
	private:
		enum ControllerIO {
			StatusRegister  = 0x0064,
			CommandRegister = 0x0064
		};
		enum StatusMask {
			OutputBuffer    = 0x01, //00000001
			InputBuffer     = 0x02, //00000010
			System          = 0x04, //00000100
			CommandData     = 0x08, //00001000
			Locked          = 0x10, //00010000
			AuxiliaryBuffer = 0x20, //00100000
			Timeout         = 0x40, //01000000
			Parity          = 0x80  //10000000
		};
	public:
		//http://www.virtualbox.org/svn/vbox/trunk/src/VBox/Devices/Input/DevPS2.cpp
		enum ControllerCommand {
			ReadCommand     = 0x20, //Read command byte
			WriteCommand    = 0x60, //Write command byte
			GetVersion      = 0xA1, //Read controller/firmware version (non-standard?)
			MouseDisable    = 0xA7, //Disable mouse port
			MouseEnable     = 0xA8, //Enable mouse port
			MouseTest       = 0xA9, //Mouse inferface test
			SelfTest        = 0xAA, //Controller test (self-test)
			KeyboardTest    = 0xAB, //Keyboard interface test
			KeyboardDisable = 0xAD, //Disable keyboard
			KeyboardEnable  = 0xAE, //Enable keyboard
			ReadInput       = 0xC0, //Read input port
			ReadOutput      = 0xD0, //Read output port
			WriteOutput     = 0xD1, //Write output port
			ReadTestInputs  = 0xE0, //Read test inputs
			SystemReset     = 0xFE, //System reset
			MouseWrite      = 0xD4  //Write to mouse
			/*Non-Standard Commands
			//0x00-0x1F //Read Controller RAM
			//0x20-0x3F //Read Controller RAM
			//0x40-0x5F //Write Controller RAM
			//0x60-0x7F //Write Controller RAM
			//0x90-0x93 //Synaptics Multiplexer Prefix
			//0x90-0x9F //Write port 13-Port 10
			0xA0 //Read Copyright
			0xA2 //Change Speed
			0xA3 //Change Speed
			0xA4 //Check if password is installed
			0xA5 //Load Password
			0xA6 //Check Password
			0xAC //Disagnostic Dump
			0xAF //Read Keyboard Version
			0xB0-0xB5 //Reset Controller Line
			0xB8-0xBD //Set Controller Line
			0xC1 //Continuous input port poll, low
			0xC2 //Continuous input port poll, high
			0xC8 //Unblock Controller lines P22 and P23
			0xC9 //Block Controller lines P22 and P23
			0xCA //Read Controller Mode
			0xCB //Write Controller Mode
			0xD2 //Write Output Buffer
			0xD3 //Write Mouse Output Buffer
			0xDD //Disable A20 address line
			0xDF //Enable A20 address line
			0xF0-0xFF //Pulse output bit*/
		};

		DevicePS2Keyboard* keyboard;
		DevicePS2Mouse* mouse;

	public:
		ControllerPS2(void);
		~ControllerPS2(void);

		//Returns true iff handled, false iff not
		bool handle_irq_keyboard(void);
		bool handle_irq_mouse(void);

		void disable_irq_keyboard(void);
		void enable_irq_keyboard(void);

		void disable_irq_mouse(void);
		void enable_irq_mouse(void);

	public:
		void send_command(ControllerCommand command);

		uint8_t read_status(void) const;

		bool is_inputbuffer_clear(void) const;
		bool is_outputbuffer_full(void) const;
		void wait_for_inputbuffer_clear(void) const;
		void wait_for_outputbuffer_full(void) const;

		bool test_self(void);
		bool test_interfaces(void);
};


}}}