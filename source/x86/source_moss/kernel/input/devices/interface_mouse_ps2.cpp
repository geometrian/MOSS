#include "interface_mouse_ps2.h"

#include "../../../mossc/_misc.h"

#include "../../graphics/vesa/controller.h"
#include "../../io/io.h"
#include "../../kernel.h"

#include "../mouse.h"

#include "controller_ps2.h"


namespace MOSS { namespace Input { namespace Devices {


//TODO: timeouts for all this!

InterfaceDevicePS2Mouse::InterfaceDevicePS2Mouse(ControllerPS2* controller, int device_index, const DeviceType& device_type) : InterfaceDevicePS2Base(controller,device_index,device_type) {
	mouse_cycle = 0;
	x = last_x = 0;
	y = last_y = 0;

	for (int i=0;i<5;++i) buttons[i]=false;
}
InterfaceDevicePS2Mouse::~InterfaceDevicePS2Mouse(void) {
}

bool InterfaceDevicePS2Mouse::handle_irq(void) /*override*/ {
	if (!InterfaceDevicePS2Base::handle_irq()) return false;

	//http://forum.osdev.org/viewtopic.php?t=10247
	//http://www.computer-engineering.org/ps2mouse/

	uint8_t byte;
	controller->recv_data(&byte);

	switch (mouse_cycle) {
		case 0:
			received_data.byte1 = byte;
			++mouse_cycle;
			//if ((received_data.byte1&0x08)!=0) {
			//	++mouse_cycle; //Only accept this as the first byte if the "must be 1" bit is set
			//}
			break;
		case 1:
			received_data.byte2 = byte;
			++mouse_cycle;
			break;
		case 2: 
			received_data.byte3 = byte;
			_handle_current_packet();
			mouse_cycle = 0;
			break;
	}

	return true;
}

void InterfaceDevicePS2Mouse::set_position(int x, int y) {
	assert_term(kernel->graphics!=nullptr&&kernel->graphics->current_mode!=nullptr,"Mouse pointer can only be operated in a graphics mode!"); //But only because we need to check where to not move it.
	if      (x <                       0) x=                         0;
	else if (x>=kernel->graphics-> width) x=kernel->graphics-> width-1;
	if      (y <                       0) y=                         0;
	else if (y>=kernel->graphics->height) y=kernel->graphics->height-1;

	int dx = x - this->x;
	int dy = y - this->y;
	if (dx!=0 || dy!=0) {
		last_x = this->x;
		last_y = this->y;
		this->x += dx;
		this->y += dy;
		kernel->handle_mouse_move(Mouse::EventMouseMove(this->x,this->y,dx,dy));
	}
}
void InterfaceDevicePS2Mouse::  click(int button_index) {
	kernel->handle_mouse_click(Mouse::EventMouseClick(button_index));
}
void InterfaceDevicePS2Mouse::unclick(int button_index) {
	kernel->handle_mouse_unclick(Mouse::EventMouseUnclick(button_index));
}
void InterfaceDevicePS2Mouse::set_sample_rate(int hz) {
	#ifdef MOSS_DEBUG
	switch (hz) {
		case 10: case 20: case 40: case 60: case 80: case 100: case 200: break;
		default:
			assert_term(false,"Invalid samping rate %d (must be one of 10, 20, 40, 60, 80, 100, 200)!",hz);
	}
	#endif

	//kernel->write(">>Sending 0xF3\n");
	send_command_device(0xF3);

	//kernel->write(">>Waiting\n");
	wait_response();

	//kernel->write(">>Sending Hz\n");
	//controller->send_data(hz);
	send_command_device(hz); //works for some reason?  See http://forum.osdev.org/viewtopic.php?f=1&t=26899
	//controller->send_data(hz);

	//kernel->write(">>Waiting\n");
	wait_response();

	//kernel->write(">>Done!\n");
}

void InterfaceDevicePS2Mouse::_handle_current_packet(void) {
	//http://wiki.osdev.org/PS/2_Mouse

	if (received_data.dx_overflowed || received_data.dy_overflowed) return; //Just give up.

	int dx = (int)(received_data.dx) - (int)((received_data.byte1<<4)&0x100);
	int dy = (int)(received_data.dy) - (int)((received_data.byte1<<3)&0x100);

	set_position(x+dx,y+dy);

	#define HANDLE_BUTTON(NAME,INDEX)\
		if (received_data.NAME && !buttons[INDEX]) {\
			buttons[INDEX] = true;\
			click(INDEX);\
		} else if (!received_data.NAME && buttons[INDEX]) {\
			buttons[INDEX] = false;\
			unclick(INDEX);\
		}
	HANDLE_BUTTON(  button_left,0)
	HANDLE_BUTTON(button_middle,1)
	HANDLE_BUTTON( button_right,2)
	#undef HANDLE_BUTTON
}


}}}