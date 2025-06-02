#pragma once

#include "../stdafx.h"

#include "misc.h"



/*
PS/2 Controller
*/



struct x86_PS2_Device
{
	DEVTYPE type;

	bool scanning_enabled;

	struct TinyRingBuffer recvbuf;
	Cycles timestamp_last_parse;

	DEVID dev_id;
};

struct x86_PS2Ctrl
{
	atomic_flag lock;
	#ifdef MOSS_DEBUG
	bool inited;
	#endif

	uint8_t num_chans;
	bool has_okay_chan[2];

	struct x86_PS2_Device devices[2];
};

extern struct x86_PS2Ctrl g_x86_ps2ctrl;



__attribute__(( no_caller_saved_registers ))
void x86_ps2_handle_irq( uint8_t irq );

/*
Initialize PS/2 controller.  Must call:
- From one core, once
- *Before* unmasking interrupts on any core
- *After* initializing USB controllers, disabling USB legacy support (otherwise we'll be
  initializing an emulated PS/2 controller!)
*/
MOSS_ND STATUS x86_ps2_init(void);

// (Re-)detect device on channel (if channel okay).  Can call after initing.
MOSS_ND STATUS x86_ps2_redetect_device( uint8_t ichan );
// (Re-)detect device(s) on any valid channel(s).  Can call after initing.
MOSS_ND STATUS x86_ps2_redetect_devices(void);

MOSS_ND STATUS x86_ps2_set_scanning( uint8_t ichan, bool enable_scanning );

void x86_ps2_handle_data(void);

// Set resolution for mouse; 0,1,2,3 -> 1,2,4,8 ticks/mm, (default 4)
MOSS_ND STATUS x86_ps2_devmouse_setres( uint8_t ichan, uint8_t res_0through3 );
// Set sample rate for mouse; 10,20,40,60,80,100,200 Hz (default 100)
MOSS_ND STATUS x86_ps2_devmouse_setrate( uint8_t ichan, uint8_t rate_hz );
