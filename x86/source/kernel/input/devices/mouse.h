#pragma once
#include "../../../includes.h"

#include "device.h"


namespace MOSS { namespace Input { namespace Devices {


//http://forum.osdev.org/viewtopic.php?t=10247
//http://houbysoft.com/download/ps2mouse.html (essentially the same?)

class ControllerPS2;

class DevicePS2Mouse : public DevicePS2Base {
	private:
		int mouse_cycle;

		int8_t mouse_byte[3];

	public:
		int x,y;
		int last_x,last_y;

	public:
		DevicePS2Mouse(ControllerPS2* controller);
		virtual ~DevicePS2Mouse(void);

		void handle_irq(void) override;

		void set_position(int x, int y);

	private:
		void send(uint16_t destination, uint8_t byte) const;
		uint8_t recv(uint16_t from) const;

	private:
		void _command(uint8_t data);
};


}}}