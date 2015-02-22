#include "keys.h"

#include "../../includes.h"
#include "../kernel.h"
#include "../input/devices/controller_ps2.h"
#include "../input/devices/keyboard.h"
#include "../../stdlib/stdlib.h"


namespace MOSS { namespace Input { namespace Keys {


bool is_printable(Codes::MossKey key) {
	switch (key) {
		#define PRINTABLE(NAME,SHIFT_NAME, DESC,SHIFT_DESC, SC) case Codes::KEY_##NAME: return strlen(DESC)==1;
		MOSS_KEY_MACRO(PRINTABLE)
		#undef MOSS_KEYS
		case Codes::KEY_INVALID: return false;
	}
	return false;
}

const char* get_printable(Codes::MossKey key) {
	Devices::DevicePS2Keyboard* keyboard = Kernel::controller->keyboard;
	ASSERT(keyboard!=NULL,"Cannot convert to a printable value; keyboard does not exist yet!");
	switch (key) {
		#define CASE_STRING(NAME,SHIFT_NAME, DESC,SHIFT_DESC, SC)\
			case SC:\
				if (keyboard->get_shift_state()) {\
					return SHIFT_DESC;\
				} else {\
					return       DESC;\
				}
		MOSS_KEY_MACRO(CASE_STRING)
		case Codes::KEY_INVALID: return "[INVALID]";
		#undef CASE_STRING
	}
	return "[UNKOWN]";
}


}}}