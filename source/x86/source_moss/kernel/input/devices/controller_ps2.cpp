#include "controller_ps2.hpp"

#include "../../io/io.hpp"
#include "../../kernel.hpp"

#include "interface_device_ps2.hpp"


namespace MOSS { namespace Input { namespace Devices {


ControllerPS2::ControllerPS2(void) {
	//http://wiki.osdev.org/%228042%22_PS/2_Controller#Initialising_the_PS.2F2_Controller (these steps):

	//Step 1: Initialize USB controllers (disabling emulation)
	//	If the system is using (typically limited/dodgy) USB Legacy Support it will emulate USB devices as PS/2 devices.  This must be
	//	disabled or we'll just be initializing the emulated PS/2 controller.
	//TODO: this

	//Step 2: Determine if the PS/2 controller exists
	//	Before touching the PS/2 controller at all, should determine if it actually exists.  On some systems (e.g. 80x86 Apple machines)
	//	it doesn't exist and any attempt to touch it can result in a system crash.  The correct way to do this is with ACPI.  Check bit
	//	1 (value=2, the "8042" flag) in the "IA PC Boot Architecture Flags" field at offset 109 in the Fixed ACPI Description Table
	//	(FADT).  If this bit is clear then there is no PS/2 Controller to configure (and MOSS exits, since its assumes one).
	//	Otherwise, if the bit is set or the system doesn't support ACPI (no ACPI tables and no FADT) then there is a PS/2 Controller.
	//TODO: this

	//Step 3: Disable devices
	{
		//Disable both ports (so that any PS/2 devices can't send data at the wrong time and mess up the initialization)
		send_command(0xAD); //No response expected
		send_command(0xA7); //No response expected (Note: if the controller is a "single channel" device, it will ignore this)
	}

	//Step 4: Flush the output buffer
	{
		//Sometimes (e.g. due to interrupt controller initialization causing a lost IRQ) data can be stuck in the PS/2 controller's output
		//	buffer.  Now that the devices are disabled (and can't send more data to it), flush the output buffer.
		if (is_outputbuffer_full()) {
			//Read from IO port 0x60 and discard the result.
			recv_data(nullptr,0);
		}
	}

	//Step 5: Disable all of the controller's IRQs and disable translation of keyboard scancodes
	{
		_ConfigurationByte config_byte = _get_configuration_byte();
		config_byte. first_port_interrupts_enabled = false;
		config_byte.second_port_interrupts_enabled = false;
		config_byte.        first_port_translation = false;
		_set_configuration_byte(config_byte);
	}

	//Step 6: Perform controller self-test
	{
		send_command(0xAA);
		uint8_t result; recv_data(&result);
		assert_term(result!=0xFC,"PS/2 Controller self-test failed!");
		assert_term(result==0x55,"PS/2 Controller self-test failed (undefined error %d)!",static_cast<int>(result));
	}

	//Step 7: Determine if there are two channels
	{
		//Enable second PS/2 port
		send_command(0xA8); //No response expected

		_ConfigurationByte config_byte = _get_configuration_byte();
		//If the following fails, then can't be a two-port PS/2 controller, since just enabled the second port.
		assert_term(!config_byte.second_port_clock_disabled,"Not a two-port PS/2 controller!");

		//Disable second PS/2 port again
		send_command(0xA7); //No response expected
	}

	//Step 8: Perform interface tests (test the PS/2 ports)
	#ifdef MOSS_DEBUG
	{
		#define CHECK_PORT(PORT)\
			recv_data(&result);\
			switch (result) {\
				case 0x00: break;\
				case 0x01: assert_term(false,"Port " #PORT " interface test failed (clock line stuck low)!");\
				case 0x02: assert_term(false,"Port " #PORT " interface test failed (clock line stuck high)!");\
				case 0x03: assert_term(false,"Port " #PORT " interface test failed (data line stuck low)!");\
				case 0x04: assert_term(false,"Port " #PORT " interface test failed (data line stuck high)!");\
				default: assert_term(false,"Port " #PORT " interface test failed (unknown)!");\
			}

		uint8_t result;
		//Test first port
		send_command(0xAB);
		CHECK_PORT(1)

		//Test second port
		send_command(0xA9);
		CHECK_PORT(2)

		#undef CHECK_PORT
	}
	#endif

	//Step 9: Enable devices
	{
		//Enable both ports
		send_command(0xAE); //No response expected
		send_command(0xA8); //No response expected
	}

	//Step 10: Reset devices
	{
		//For each device:
		//	Reset device
		//	Get the device's id
		//	Allocate a subclass based on the particular id that handles the device
		//	Return as base pointer
		device0 = InterfaceDevicePS2Base::get_new_device(this, 0);
		device1 = InterfaceDevicePS2Base::get_new_device(this, 1);
	}

	//Step 11: Enable interrupts
	//	OSDev suggests this as part of step 9, but I think it should come after the devices have been set up properly.
	{
		_ConfigurationByte config_byte = _get_configuration_byte();
		config_byte. first_port_interrupts_enabled = true;
		config_byte.second_port_interrupts_enabled = true;
		_set_configuration_byte(config_byte);
	}

	//Step 12: Enable devices' data streams
	{
		device0->enable();
		device1->enable();
	}

	//kernel->write("Keyboard using scancode "); kernel->write((int)(keyboard->get_scancode())); kernel->write("!\n");
	//assert_term(keyboard->set_scancode(2)==2,"Could not set keyboard scancode to 2!");
}
ControllerPS2::~ControllerPS2(void) {
	delete device0;
	delete device1;
}

void ControllerPS2::send_command(uint8_t command) {
	wait_for_inputbuffer_clear();
	IO::send<uint8_t>(CommandRegister,command);
}
void ControllerPS2::send_data(uint8_t data) {
	wait_for_inputbuffer_clear();
	IO::send<uint8_t>(DataPort,data);
}
bool ControllerPS2::recv_data(uint8_t* data, int timeout_counter/*=-1*/) {
	if (timeout_counter==-1) {
		wait_for_outputbuffer_full();
	} else {
		while (timeout_counter>0) {
			if (is_outputbuffer_full()) goto RECV;
			--timeout_counter;
		}
		return false;
	}

	RECV:
		uint8_t result = IO::recv<uint8_t>(DataPort);
		if (data!=nullptr) *data=result;

	return true;
}

bool ControllerPS2::is_inputbuffer_full(void) const {
	return _get_status_byte().input_buffer_full;
}
bool ControllerPS2::is_outputbuffer_full(void) const {
	return _get_status_byte().output_buffer_full;
}
void ControllerPS2::wait_for_inputbuffer_clear(void) const {
	while (is_inputbuffer_full());
}
void ControllerPS2::wait_for_outputbuffer_full(void) const {
	while (!is_outputbuffer_full());
}

ControllerPS2::_StatusByte ControllerPS2::_get_status_byte(void) const {
	_StatusByte status_byte;

	status_byte.data_byte = IO::recv<uint8_t>(StatusRegister);

	return status_byte;
}

ControllerPS2::_ConfigurationByte ControllerPS2::_get_configuration_byte(void) {
	_ConfigurationByte config_byte;

	send_command(0x20);
	recv_data(&config_byte.data_byte);

	return config_byte;
}
void ControllerPS2::_set_configuration_byte(_ConfigurationByte const& config_byte) {
	send_command(0x60); //No response expected
	send_data(config_byte.data_byte);
}


}}}
