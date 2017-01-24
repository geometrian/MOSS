#include "interface_device_ps2.hpp"

#include "../../io/io.hpp"
#include "../../kernel.hpp"

#include "controller_ps2.hpp"
#include "interface_keyboard_ps2.hpp"
#include "interface_mouse_ps2.hpp"


namespace MOSS { namespace Input { namespace Devices {


InterfaceDevicePS2Base::InterfaceDevicePS2Base(ControllerPS2* controller, int device_index, DeviceType const& device_type) : controller(controller), device_index(device_index), device_type(device_type) {
}
InterfaceDevicePS2Base::~InterfaceDevicePS2Base(void) {
}

InterfaceDevicePS2Base* InterfaceDevicePS2Base::get_new_device(ControllerPS2* controller, int device_index) {
	//http://wiki.osdev.org/%228042%22_PS/2_Controller#Detecting_PS.2F2_Device_Types
	//Following my idea from my post here: http://forum.osdev.org/viewtopic.php?f=1&t=26889

	kernel->write_sys(2,"Checking PS/2 device (index %d):\n",device_index);

	bool attempted_ps2_mouse_config1 = false;
	bool attempted_ps2_mouse_config2 = false;
	START:

	//Reset device
	{
		//"The mouse probably sends ACK (0xFA) plus several more bytes, then resets itself, and always sends 0xAA."
		//This is better: http://www.computer-engineering.org/ps2mouse/ sends some bytes, then 0xAA, then device id 0x00.
		send_command_device(controller,device_index, 0xFF);

		/*uint8_t response;
		do {
			bool result = controller->recv_data(&response,10000);
			ASSERT(result,"PS/2 mouse does not exist or reset failed!");
		} while (response!=0xAA);*/
		uint8_t response;
		bool got_affirm = false;
		while (controller->recv_data(&response,500000)) {
			//kernel->write("%d ",response);
			if (response==0xAA) got_affirm=true;
		}
		assert_term(got_affirm,"  PS/2 device (index %d) does not exist or reset failed!",device_index);
	}

	//Get device's ID and new device subclass instance based off of it
	InterfaceDevicePS2Base* new_device = nullptr;
	{
		//http://wiki.osdev.org/%228042%22_PS/2_Controller#Detecting_PS.2F2_Device_Types
		//Asks the PS/2 device to identify itself -> 0xFA(ACK),[none, 1, or 2 bytes]
		//Expects no data bytes
		send_command_device(controller,device_index, 0xF2);
		uint8_t response, byte1,byte2;
		controller->recv_data(&response); assert_term(response==0xFA,"  Device ID ACK failed!"); //http://wiki.osdev.org/PS/2_Keyboard#Special_Bytes seems to imply that this can only return ACK?
		if (controller->recv_data(&byte1, 10000)) {
			if (controller->recv_data(&byte2, 10000)) {
				assert_term(byte1==0xAB,"  Unrecognized PS/2 Device (device id %u %u)!",byte1,byte2);
				switch (byte2) {
					case 0x41:
					case 0xC1:
						//MF2 keyboard with translation enabled in the PS/Controller (not possible for the second PS/2 port)
						kernel->write_sys(3,"PS/2 device %d (id %u %u) recognized as MF2 keyboard (with translation)!\n",device_index,byte1,byte2);
						new_device = new InterfaceDevicePS2Keyboard(controller,device_index,DeviceType::DeviceKeyboardMF2Trans);
						break;
					case 0x83:
						//MF2 keyboard
						kernel->write_sys(3,"PS/2 device %d (id %u %u) recognized as MF2 keyboard!\n",device_index,byte1,byte2);
						new_device = new InterfaceDevicePS2Keyboard(controller,device_index,DeviceType::DeviceKeyboardMF2);
						break;
					default:
						assert_term(byte1==0xAB,"  PS/2 Device %d (id %u %u) unrecognized!",device_index,byte1,byte2);
				}
			}
			else {
				switch (byte1) {
					case 0x00: //Standard PS/2 mouse
						//Awww sadface.  Let's configure it and try again.
						kernel->write_sys(3,"PS/2 device %d (id %u) recognized as basic mouse!\n",device_index,byte1);
						new_device = new InterfaceDevicePS2Mouse(controller,device_index,DeviceType::DeviceMouseBasic);
						if (!attempted_ps2_mouse_config1) { //but only if we haven't tried that before
							//http://wiki.osdev.org/Mouse_Input#Init.2FDetection_Command_Sequences
							kernel->write_sys(3,"Checking to see if it is actually better . . .\n");
							static_cast<InterfaceDevicePS2Mouse*>(new_device)->set_sample_rate(200);
							static_cast<InterfaceDevicePS2Mouse*>(new_device)->set_sample_rate(100);
							static_cast<InterfaceDevicePS2Mouse*>(new_device)->set_sample_rate( 80);
							delete new_device;
							attempted_ps2_mouse_config1 = true;
							goto START;
						}
						break;
					case 0x03: //Mouse with scroll wheel
						//Less of a sadface, but still.  Let's configure it and try again.
						kernel->write_sys(3,"PS/2 device %d (id %u) recognized as scrollwheel mouse!\n",device_index,byte1);
						new_device = new InterfaceDevicePS2Mouse(controller,device_index,DeviceType::DeviceMouseScroll);
						if (!attempted_ps2_mouse_config2) { //but only if we haven't tried that before
							//http://wiki.osdev.org/Mouse_Input#Init.2FDetection_Command_Sequences
							kernel->write_sys(3,"Checking to see if it is actually better . . .\n");
							static_cast<InterfaceDevicePS2Mouse*>(new_device)->set_sample_rate(200);
							static_cast<InterfaceDevicePS2Mouse*>(new_device)->set_sample_rate(200);
							static_cast<InterfaceDevicePS2Mouse*>(new_device)->set_sample_rate( 80);
							attempted_ps2_mouse_config2 = true;
							goto START;
						}
						break;
					case 0x04: //5-button mouse
						kernel->write_sys(3,"PS/2 device %d (id %u) recognized as five-button mouse!\n",device_index,byte1);
						new_device = new InterfaceDevicePS2Mouse(controller,device_index,DeviceType::DeviceMouse5);
						break;
					default:
						assert_term(false,"  PS/2 Device %d (id %u) unrecognized!",device_index,byte1);
						break;
				}
			}
		} else {
			//None: Ancient AT keyboard with translation enabled in the PS/Controller (not possible for the second PS/2 port)
			assert_term(false,"  Assuming PS/2 device %d (id none) is AT keyboard; not supported!",device_index);
		}
	}

	//Set to default values, and then the values that MOSS wants.
	{
		if (new_device==nullptr) assert_term(false,"PS/2 device was null somehow!");
		new_device->set_defaults();
		switch (new_device->device_type) {
			case DeviceKeyboardMF2:
			case DeviceKeyboardMF2Trans:
				static_cast<InterfaceDevicePS2Keyboard*>(new_device)->set_scancode(1); //TODO: lame!
				break;
			case DeviceMouseBasic:
			case DeviceMouseScroll:
			case DeviceMouse5:
				break;
			default:
				assert_term(false,"Unrecognized device type!");
		}
	}

	return new_device;
}

void InterfaceDevicePS2Base::set_defaults(void) {
	//Tell the device, whatever it may be, to use its default settings
	//	Mouse: disables streaming, sets the packet rate to 100 per second and resolution to 4 pixels per mm.
	//	Keyboard: Typematic delay 500 ms.  Typematic rate 10.9 cps.  Scan code set 2.  Set all keys typematic/make/break.
	//		See http://www.computer-engineering.org/ps2keyboard/
	send_command_device(0xF6);
	wait_response();
}

bool InterfaceDevicePS2Base::handle_irq(void) {
	//Check that the interrupt was genuine
	if (!controller->is_outputbuffer_full()) return false;

	return true;
}

void InterfaceDevicePS2Base:: enable(void) {
	//Enables the device to automatically send data about itself; to be called
	//after setup and after after interrupts are enabled.
	//	Mouse: Enable packet streaming
	//	Keyboard: Enable scanning
	send_command_device(0xF4);
	wait_response();
}
void InterfaceDevicePS2Base::disable(void) {
	//	Mouse: Disable packet streaming
	//	Keyboard: Disable scanning (Note: may also restore default parameters)
	//		TODO: OSDev seems to think that 0xF5 disables scanning, possibly also resetting to default parameters, whereas http://www.brokenthorn.com/Resources/OSDev19.html
	//		seems to think that 0xF5 resets to power on condition and then waits for the enable command.  I.e., osdev thinks the reset may happen, BrokenThorn avers it will.
	send_command_device(0xF5);
	wait_response();
}

//List of commands: http://wiki.osdev.org/Mouse_Input#Useful_Mouse_Command_Set
void InterfaceDevicePS2Base::send_command_device(uint8_t command) {
	send_command_device(controller,device_index, command);
}
void InterfaceDevicePS2Base::send_command_device(ControllerPS2* controller,int device_index, uint8_t command) {
	//kernel->write("Sending device command %d\n",command);

	//Commands are sent on port 0x60, and by default go to device 0.  To send a command
	//to device 1 instead, we prefix the command with a byte telling the PS/2 controller
	//to do so.  See http://www.computer-engineering.org/ps2keyboard/ for device 0.

	if (device_index==1) {
		//Tell the PS/2 controller that the next byte should be redirected to the device 1
		controller->send_command(0xD4); //No response expected
	}

	//Write command to PS/2 controller, which will now redirect it to the device port
	controller->send_data(command); //Caller should check for a response, iff applicable!
}
void InterfaceDevicePS2Base::wait_response(uint8_t wait_byte/*=0xFA*/) {
	wait_response(controller,wait_byte);
}
void InterfaceDevicePS2Base::wait_response(ControllerPS2* controller, uint8_t wait_byte/*=0xFA*/) {
	//Generally called after most commands.
	//	For mice, most commands return 0xFA (ACK).  The only one that (might) not is the reset command.
	uint8_t response;
	controller->recv_data(&response);
	assert_term(response==wait_byte,"Command to device failed (expected %d, got %d)!",wait_byte,response);
}


}}}
