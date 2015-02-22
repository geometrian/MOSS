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

		//http://www.computer-engineering.org/ps2mouse/
		union {
			struct {
				bool    button_left : 1;
				bool   button_right : 1;
				bool  button_middle : 1;
				bool         unused : 1; //Should be 1
				uint32_t sign_bit_x : 1; //Two's complement
				uint32_t sign_bit_y : 1; //Two's complement
				bool   overflowed_x : 1;
				bool   overflowed_y : 1;
				uint32_t         dx : 8;
				uint32_t         dy : 8;
				int32_t      dwheel : 4; //Only valid for Intellimouse Extensions
				bool       button_4 : 1; //Only valid for Intellimouse Extensions (and only iff initialized extra)
				bool       button_5 : 1; //Only valid for Intellimouse Extensions (and only iff initialized extra)
			};
			struct {
				uint32_t      byte1 : 8;
				uint32_t      byte2 : 8;
				uint32_t      byte3 : 8;
				uint32_t      byte4 : 8; //Only used for Intellimouse Extensions
			};
		} received_data;
		//uint8_t mouse_data[3];

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