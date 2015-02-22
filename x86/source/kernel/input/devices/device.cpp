#include "device.h"

#include "../../io/io.h"

#include "controller_ps2.h"


namespace MOSS { namespace Input { namespace Devices {


DevicePS2Base::DevicePS2Base(ControllerPS2* controller) : controller(controller) {
}
DevicePS2Base::~DevicePS2Base(void) {
}


}}}