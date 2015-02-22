#include "mouse.h"

#include "../../graphics/vesa/controller.h"
#include "../../io/io.h"
#include "../../kernel.h"

#include "../mouse.h"

#include "controller_ps2.h"


#include "../../../mossc/_misc.h"

#include "../../kernel.h"
#include "../../text_mode_terminal.h"


namespace MOSS { namespace Input { namespace Devices {


//TODO: timeouts for all this!

DevicePS2Mouse::DevicePS2Mouse(ControllerPS2* controller) : DevicePS2Base(controller) {
	ASSERT(sizeof(received_data)==4,"Mouse data packet is the wrong size!");

	mouse_cycle = 0;
	x = last_x = 0;
	y = last_y = 0;

	for (int i=0;i<5;++i) buttons[i]=false;
}
DevicePS2Mouse::~DevicePS2Mouse(void) {
}

void DevicePS2Mouse::reset(void) {
	//The mouse probably sends ACK (0xFA) plus several more bytes, then resets itself, and always sends 0xAA.
	send_command_device(0xFF);

	/*uint8_t response;
	do {
		bool result = controller->recv_data(&response,10000);
		ASSERT(result,"PS/2 mouse does not exist or reset failed!");
	} while (response!=0xAA);*/
	uint8_t response;
	bool got_affirm = false;
	while (controller->recv_data(&response,10000)) if (response==0xAA) got_affirm=true;
	ASSERT(got_affirm,"PS/2 mouse does not exist or reset failed!");

	//Tell the mouse to use default settings
	//	Disables streaming, sets the packet rate to 100 per second, and resolution to 4 pixels per mm.
	send_command_device(0xF6);
	wait_response();
}

void DevicePS2Mouse:: enable_streaming(void) {
	//Enable packet streaming
	send_command_device(0xF4);
	wait_response();
}
void DevicePS2Mouse::disable_streaming(void) {
	//Disable packet streaming
	send_command_device(0xF5);
	wait_response();
}

void DevicePS2Mouse::set_position(int x, int y) {
	ASSERT(Kernel::graphics!=NULL&&Kernel::graphics->current_mode!=NULL,"Mouse pointer can only be operated in a graphics mode!"); //But only because we need to check where to not move it.
	if      (x <                       0) x=                         0;
	else if (x>=Kernel::graphics-> width) x=Kernel::graphics-> width-1;
	if      (y <                       0) y=                         0;
	else if (y>=Kernel::graphics->height) y=Kernel::graphics->height-1;

	int dx = x - this->x;
	int dy = y - this->y;
	if (dx!=0 || dy!=0) {
		last_x = this->x;
		last_y = this->y;
		this->x += dx;
		this->y += dy;
		Kernel::handle_mouse_move(Mouse::EventMouseMove(this->x,this->y,dx,dy));
	}
}
void DevicePS2Mouse::  click(int button_index) {
	Kernel::handle_mouse_click(Mouse::EventMouseClick(button_index));
}
void DevicePS2Mouse::unclick(int button_index) {
	Kernel::handle_mouse_unclick(Mouse::EventMouseUnclick(button_index));
}

void DevicePS2Mouse::handle_irq(void) /*override*/ {
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
}

//List of commands: http://wiki.osdev.org/Mouse_Input#Useful_Mouse_Command_Set
void DevicePS2Mouse::send_command_device(uint8_t command) {
	Kernel::terminal->write("Sending mouse command %d\n",command);

	//Tell the PS/2 controller that the next byte should be redirected to the second port (mouse)
	controller->send_command(0xD4); //No response expected

	//Write command to PS/2 controller, which will now redirect it to the mouse
	controller->send_data(command); //Caller should check for a response, iff applicable!
}
void DevicePS2Mouse::wait_response(uint8_t wait_byte/*=0xFA*/) {
	//Called after most commands, which return 0xFA (ACK).  The only one that (might) not is the reset command.
	uint8_t response;
	controller->recv_data(&response);
	ASSERT(response==wait_byte,"Command to mouse failed (expected %d, got %d)!",wait_byte,response);
}

void DevicePS2Mouse::_handle_current_packet(void) {
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