#include "device.h"

#include "../../io/io.h"

#include "controller_ps2.h"


namespace MOSS { namespace Input { namespace Devices {


DevicePS2Base::DevicePS2Base(ControllerPS2* controller) : controller(controller) {
}
DevicePS2Base::~DevicePS2Base(void) {
}

void DevicePS2Base::send(uint8_t byte) const {
	//Since commands sent to the device are sent to the controller first, the controller must be
	//ready for the command (at least for the keyboard encoder?)
	controller->wait_for_inputbuffer_clear();
	IO::send<uint8_t>(get_command_register(),byte);
}
//Read device (e.g. keyboard encoder) buffer
uint8_t DevicePS2Base::recv(void) const {
	controller->wait_for_outputbuffer_full();
	return IO::recv<uint8_t>(get_inputbuffer());
}


}}}