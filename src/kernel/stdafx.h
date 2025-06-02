#pragma once

#include "../libmoss-util/include.h"



#define kassert( PASS_COND, FMT_CSTR,... )\
	__moss_assert( PASS_COND, FMT_CSTR __VA_OPT__(,) __VA_ARGS__ )



// Status codes.  0 is success, negative are errors.
typedef enum
{
	STATUS_Success = 0,

	STATUS_x86_PS2_TestFailedCtrl          = -1,
	STATUS_x86_PS2_TestFailedCtrlBothPorts = -2,
	STATUS_x86_PS2_TestFailedDevice        = -3,
	STATUS_x86_PS2_Timeout                 = -4,
	STATUS_x86_PS2_DeviceCommError         = -5,
	STATUS_x86_PS2_DeviceError             = -6,
	STATUS_x86_PS2_BadChannel              = -7,
}
STATUS;



// Device types.  0 is none, negative are errors.
typedef enum
{
	DEVTYPE_InputKeyboard = 1,
	DEVTYPE_InputMouse    = 2,

	DEVTYPE_x86_PS2_None               =  0,
	DEVTYPE_x86_PS2_Unknown            = -1,
	DEVTYPE_x86_PS2_Broken             = -2,
	DEVTYPE_x86_PS2_Keyboard           =  1,
	DEVTYPE_x86_PS2_Mouse              =  2,
	DEVTYPE_x86_PS2_MouseScroll        =  3,
	DEVTYPE_x86_PS2_MouseScrollButtons =  4
}
DEVTYPE;

typedef uint32_t DEVID;



typedef uint64_t Cycles;
