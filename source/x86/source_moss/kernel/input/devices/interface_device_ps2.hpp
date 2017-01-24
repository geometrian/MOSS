#pragma once

#include "../../../includes.hpp"


namespace MOSS { namespace Input { namespace Devices {


class ControllerPS2;

//Handles interfacing with a PS/2 device

class InterfaceDevicePS2Base {
	protected:
		enum DeviceType {
			DeviceMouseBasic, //Standard PS/2 mouse
			DeviceMouseScroll, //Mouse with scrollwheel
			DeviceMouse5, //Five-button mouse
			DeviceKeyboardMF2, //MF2 keyboard
			DeviceKeyboardMF2Trans, //MF2 keyboard with (scancode set?) translation enabled in the PS/Controller (not possible for the second PS/2 port)
		};

		ControllerPS2*const controller;
		const int device_index;
		const DeviceType device_type;

	protected:
		InterfaceDevicePS2Base(ControllerPS2* controller, int device_index, DeviceType const& device_type);
	public:
		virtual ~InterfaceDevicePS2Base(void);

		static InterfaceDevicePS2Base* get_new_device(ControllerPS2* controller, int device_index);

		void set_defaults(void);

		virtual bool handle_irq(void);

		void  enable(void);
		void disable(void);

		       void send_command_device(uint8_t command);
		static void send_command_device(ControllerPS2* controller,int device_index, uint8_t command);
		       void wait_response(uint8_t wait_byte=0xFA);
		static void wait_response(ControllerPS2* controller, uint8_t wait_byte=0xFA);
};


}}}
