#include "controller_ps2.h"

#include "../../io/io.h"

#include "keyboard.h"
#include "mouse.h"


namespace MOSS { namespace Input { namespace Devices {


ControllerPS2::ControllerPS2(void) {
	//TODO: http://wiki.osdev.org/%228042%22_PS/2_Controller#Initialising_the_PS.2F2_Controller

	keyboard = new DevicePS2Keyboard(this);
	//Kernel::terminal->write("Keyboard using scancode "); Kernel::terminal->write((int)(keyboard->get_scancode())); Kernel::terminal->write("!\n");
	//ASSERT(keyboard->set_scancode(2)==2,"Could not set keyboard scancode to 2!");
	//ASSERT(test(),"PS/2 Controller test failed!");

	mouse = new DevicePS2Mouse(this);
}
ControllerPS2::~ControllerPS2(void) {
	delete mouse;

	delete keyboard;
}

bool ControllerPS2::handle_irq_keyboard(void) {
	//Check that the interrupt was genuine
	if (!is_outputbuffer_full()) return false;

	keyboard->handle_irq();

	return true;
}
bool ControllerPS2::handle_irq_mouse(void) {
	//Check that the interrupt was genuine
	if (!is_outputbuffer_full()) return false;

	mouse->handle_irq();

	return true;
}

void ControllerPS2::disable_irq_keyboard(void) {
	send_command(KeyboardDisable);
}
void ControllerPS2::enable_irq_keyboard(void) {
	send_command(KeyboardEnable);
}

void ControllerPS2::disable_irq_mouse(void) {
	send_command(MouseDisable);
}
void ControllerPS2::enable_irq_mouse(void) {
	send_command(MouseEnable);
}

void ControllerPS2::send_command(ControllerCommand command) {
	wait_for_inputbuffer_clear();
	IO::send<uint8_t>(CommandRegister,command);
}

uint8_t ControllerPS2::read_status(void) const {
	return IO::recv<uint8_t>(StatusRegister);
}

bool ControllerPS2::is_inputbuffer_clear(void) const {
	return !(read_status() & InputBuffer);
}
bool ControllerPS2::is_outputbuffer_full(void) const {
	return read_status() & OutputBuffer;
}
void ControllerPS2::wait_for_inputbuffer_clear(void) const {
	//TODO: IO::wait()?
	while (true) {
		if (is_inputbuffer_clear()) {
			break;
		}
	}
}
void ControllerPS2::wait_for_outputbuffer_full(void) const {
	//TODO: IO::wait()?
	while (true) {
		if (is_outputbuffer_full()) {
			break;
		}
	}
}

bool ControllerPS2::test_self(void) {
	send_command(SelfTest);
	wait_for_outputbuffer_full();

	//0x55=success, 0xFC=failure
	return keyboard->recv()==0x55;
}
bool ControllerPS2::test_interfaces(void) {
	send_command(KeyboardTest);
	wait_for_outputbuffer_full();

	//0x00 = Success; no errors
	//0x01 = Keyboard clock line stuck low
	//0x02 = Keyboard clock line stuck high
	//0x03 = Keyboard data line stuck high
	//0xFF = General error
	return keyboard->recv()==0x00;

	//TODO: also test mouse!
}


}}}