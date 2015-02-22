#pragma once
#include "../../../includes.h"


namespace MOSS { namespace Input { namespace Devices {


class ControllerPS2;

class DevicePS2Base {
	protected:
		enum DeviceType {
			DeviceMouse, //Standard PS/2 mouse
			DeviceMouseScroll, //Mouse with scrollwheel
			DeviceMouse5, //Five-button mouse
			DeviceKeyboardMF2, //MF2 keyboard
			DeviceKeyboardMF2Trans, //MF2 keyboard with (scancode set?) translation enabled in the PS/Controller (not possible for the second PS/2 port) 
		} device_type;

		ControllerPS2*const controller;

	protected:
		DevicePS2Base(ControllerPS2* controller);
	public:
		virtual ~DevicePS2Base(void);

		virtual void handle_irq(void) = 0;
};


}}}