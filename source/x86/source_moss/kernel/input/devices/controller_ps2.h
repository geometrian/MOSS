#pragma once

#include "../../../includes.h"

#define MOSS_MAX_PS2_ATTEMPTS 3


namespace MOSS { namespace Input { namespace Devices {


//This class encapsulates the PS/2 controller, (sometimes referred to elsewhere as "keyboard controller"; a
//misnomer since the PS/2 controller controls up to two devices).  This is a good diagram of the setup:
//http://wiki.osdev.org/%228042%22_PS/2_Controller#Overview.


class InterfaceDevicePS2Base;

//http://wiki.osdev.org/%228042%22_PS/2_Controller
class ControllerPS2 final {
	private:
		enum _ControllerIO {
			DataPort        = 0x0060u,
			StatusRegister  = 0x0064u,
			CommandRegister = 0x0064u
		};

		class _StatusByte final { public:  //"Compaq status byte" (not actually Compaq-specific (anymore).
			union {
				struct {
					bool                output_buffer_full : 1;
					bool                 input_buffer_full : 1;
					uint8_t                    system_flag : 1;
					uint8_t input_for_device_or_controller : 1; //Writes to input buffer for 0=PS/2 device, 1=PS/2 controller
					uint8_t                        unknown : 1; //May be keyboard locked
					uint8_t         data_from_which_device : 1; //http://wiki.osdev.org/Mouse_Input#Keyboard.2FAux_Data_Bit (0=keyboard, 1=mouse) //May be "receive time-out" or "second PS/2 port output buffer full" (first link implies it is definitely the second)
					bool                     timeout_error : 1;
					uint8_t                   parity_error : 1;
				};
				uint8_t data_byte;
			};
		};
		static_assert(sizeof(_StatusByte)==sizeof(uint8_t),"Status byte was the wrong size!");

		class _ConfigurationByte final { public:
			union {
				struct {
					bool  first_port_interrupts_enabled : 1;
					bool second_port_interrupts_enabled : 1;
					bool                    system_flag : 1;
					uint8_t                       zero1 : 1;
					bool      first_port_clock_disabled : 1;
					bool     second_port_clock_disabled : 1;
					bool         first_port_translation : 1; //from scan code set n to scan code set 1?
					uint8_t                       zero2 : 1;
				};
				uint8_t data_byte;
			};
		};
		static_assert(sizeof(_ConfigurationByte)==sizeof(uint8_t),"Configuration byte was the wrong size!");

	public:
#if 0
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
#endif

		InterfaceDevicePS2Base* device0;
		InterfaceDevicePS2Base* device1;

	public:
		ControllerPS2(void);
		~ControllerPS2(void);

	public:
		void send_command(uint8_t command);
		void send_data(uint8_t data);
		bool recv_data(uint8_t* data, int timeout_counter=-1);

		bool is_inputbuffer_full(void) const;
		bool is_outputbuffer_full(void) const;
		void wait_for_inputbuffer_clear(void) const;
		void wait_for_outputbuffer_full(void) const;

	private:
		_StatusByte _get_status_byte(void) const;

		_ConfigurationByte _get_configuration_byte(void);
		void _set_configuration_byte(_ConfigurationByte const& config_byte);
};


}}}
