#include "ps2.h"

#include "../device/inpdev.h"

#include "../kernel.h"



/*
More resources:

Controller:
	https://wiki.osdev.org/%228042%22_PS/2_Controller
	https://wiki.osdev.org/%228042%22_PS/2_Controller#Detecting_PS.2F2_Device_Types
	https://wiki.osdev.org/%228042%22_PS/2_Controller#Initialising_the_PS.2F2_Controller
	https://wiki.osdev.org/%228042%22_PS/2_Controller#Interrupts
	https://wiki.osdev.org/%228042%22_PS/2_Controller#Receiving_Bytes_From_Device.2Fs
	https://wiki.osdev.org/%228042%22_PS/2_Controller#Sending_Bytes_To_Device.2Fs
	https://wiki.osdev.org/%228042%22_PS/2_Controller#USB_Legacy_Support
	https://wiki.osdev.org/Mouse_Input#Keyboard.2FAux_Data_Bit

https://www.virtualbox.org/svn/vbox/trunk/src/VBox/Devices/Input/DevPS2.cpp

Keyboard:
	https://web.archive.org/web/20180217074705/http://www.computer-engineering.org:80/ps2keyboard
	https://wiki.osdev.org/PS/2_Keyboard
	https://wiki.osdev.org/PS/2_Keyboard#Commands
	https://wiki.osdev.org/PS/2_Keyboard#Special_Bytes
	http://www.brokenthorn.com/Resources/OSDev19.html
	https://learn.microsoft.com/en-us/previous-versions/windows/embedded/ms894073(v=msdn.10)
	http://www.brokenthorn.com/Resources/OSDevScanCodes.html

Mouse:
	https://wiki.osdev.org/Mouse_Input
	https://wiki.osdev.org/Mouse_Input#Init.2FDetection_Command_Sequences
	https://wiki.osdev.org/Mouse_Input#Useful_Mouse_Command_Set
	https://wiki.osdev.org/PS/2_Mouse
	https://web.archive.org/web/20180202180653/http://www.computer-engineering.org/ps2mouse/
	https://web.archive.org/web/20041117095622/http://panda.cs.ndsu.nodak.edu/~achapwes/PICmicro/mouse/mouse.html
	https://forum.osdev.org/viewtopic.php?t=10247
	https://forum.osdev.org/viewtopic.php?t=26899
*/



//Max time the controller can take to accept command
#define TIMEOUTMS_CTRL_CMD_ACPT 5
//Max time the controller can take to execute and return a value from a command
#define TIMEOUTMS_CTRL_CMD_EXEC 5
//Max time a device can take to accept command
#define TIMEOUTMS_DEV_CMD_ACPT 5
//Max time a device can take to process command
//	VirtualBox's keyboard wants at least ≈33,000,000 cycles (counted by `rdtsc`) to self-test.  Give
//	it a luxurious 50 ms (50 Mcycles if ≥1 GHz).
#define TIMEOUTMS_DEV_CMD_EXEC 50
//Max time to wait for packet to arrive
//	VirtualBox's keyboard needs more than 100ms
#define TIMEOUTMS_DEV_PACKET 1000

#ifdef MOSS_DEBUG
	//#define ENABLE_RW_LOGGING
	bool _g_log_enabled = false;
#endif



// https://wiki.osdev.org/%228042%22_PS/2_Controller#PS.2F2_Controller_Configuration_Byte
typedef union
{
	struct
	{
		uint8_t interrupt_enable_chan1   : 1;
		uint8_t interrupt_enable_chan2   : 1;
		uint8_t sys_flag                 : 1; // set by firmware if passes self-tests
		uint8_t zero1                    : 1; // zero
		uint8_t disable_clock_chan1      : 1;
		uint8_t disable_clock_chan2      : 1;
		uint8_t enable_translation_chan1 : 1;
		uint8_t zero2                    : 1; // zero
	};
	uint8_t packed;
}
_Config;
static_assert( sizeof(_Config) == 1 );

// https://wiki.osdev.org/%228042%22_PS/2_Controller#Status_Register
// "Compaq status byte" (not actually Compaq-specific (anymore)).
typedef union
{
	struct
	{
		uint8_t ctrl_buf_out_full  : 1; // controller's data 'output buffer' full
		uint8_t ctrl_buf_inp_full  : 1; // controller's data 'input buffer' full
		uint8_t sys_flag           : 1; // set by firmware if passes self-tests
		uint8_t inp_for_0dev_1ctrl : 1; // whether input is for the device or the controller
		uint8_t                    : 1; // chipset-specific
		uint8_t data_from_dev_0or1 : 1; // chipset-specific, see https://wiki.osdev.org/Mouse_Input#Keyboard.2FAux_Data_Bit
		uint8_t err_timeout        : 1;
		uint8_t err_parity         : 1;
	};
	uint8_t packed;
}
_Status;
static_assert( sizeof(_Status) == 1 );



struct x86_PS2Ctrl g_x86_ps2ctrl;



//Lowest-Level API: reading and writing

//	Read data (threadsafe and interrupt-safe)
__attribute__(( no_caller_saved_registers ))
MOSS_ND_INLINE static uint8_t _r_data  (void)
{
	// Read from 0x0060 is data
	uint8_t data = x86_io_in8( 0x0060 );
	#ifdef ENABLE_RW_LOGGING
	if (_g_log_enabled) klogf( "[R DAT->%02x]", data );
	#endif
	return data;
}
//	Read status (threadsafe and interrupt-safe)
MOSS_ND_INLINE static _Status _r_status(void)
{
	// Read from 0x0064 is status
	uint8_t val = x86_io_in8( 0x0064 );
	#ifdef ENABLE_RW_LOGGING
	if (_g_log_enabled) klogf( "[R STA->%02x]", val );
	#endif
	return (_Status){ .packed=val };
}
//	Wait for the data register to have something on it.
//		NOT threadsafe (race condition)
//		NOT interrupt-safe (IRQ would try to handle the data we're waiting on)
MOSS_ND static STATUS _r_wait_data_unsafe( uint32_t timeout_ms, uint8_t* data )
{
	if ( _r_status().ctrl_buf_out_full ) goto GOT;

	Cycles start = x86_rdtsc_nofences();
	Cycles end = start + kernel_conv_delayms_to_cycles( timeout_ms );
	LOOP:
		if ( !_r_status().ctrl_buf_out_full )
		{
			Cycles now = x86_rdtsc_nofences();
			if ( now >= end ) return STATUS_x86_PS2_Timeout;
			__builtin_ia32_pause();
			goto LOOP;
		}

	GOT:
		*data = _r_data();

		return STATUS_Success;
}
//	Wait for the controller's input buffer to be empty, meaning the byte has been consumed by the
//	controller or device (threadsafe and interrupt-safe).
MOSS_ND static STATUS _w_wait_sent( uint32_t timeout_ms )
{
	if ( !_r_status().ctrl_buf_inp_full ) return STATUS_Success;

	Cycles start = x86_rdtsc_nofences();
	Cycles end = start + kernel_conv_delayms_to_cycles( timeout_ms );
	LOOP:
		if ( _r_status().ctrl_buf_inp_full )
		{
			Cycles now = x86_rdtsc_nofences();
			if ( now >= end ) return STATUS_x86_PS2_Timeout;
			__builtin_ia32_pause();
			goto LOOP;
		}

	return STATUS_Success;
}
//	Write data
//		NOT threadsafe (other thread could write buffer, filling it so we lose our data)
//		IS interrupt-safe (interrupt doesn't care about this)
MOSS_ND_INLINE STATUS _w_data_unsafe( uint8_t data, uint32_t timeout_ms )
{
	// Write to 0x0060 is data
	#ifdef ENABLE_RW_LOGGING
	if (_g_log_enabled) klogf( "[W DAT<-%02x]", data );
	#endif
	x86_io_out8( 0x0060, data );

	return _w_wait_sent( timeout_ms );
}
//	Write command
//		NOT threadsafe (other thread could write buffer, filling it so we lose our data)
//		IS interrupt-safe (interrupt doesn't care about this)
MOSS_ND_INLINE STATUS _w_cmd_unsafe ( uint8_t cmd , uint32_t timeout_ms )
{
	// Write to 0x0064 is command
	#ifdef ENABLE_RW_LOGGING
	if (_g_log_enabled) klogf( "[W CMD<-%02x]", cmd );
	#endif
	x86_io_out8( 0x0064, cmd );

	return _w_wait_sent( timeout_ms );
}



//Lower-Level API: commands and basic functions

//	Controller command: 1 or 2 byte commands, expecting 0 or 1 byte responses
//		NOT threadsafe (other thread could write buffer, filling it so we lose our data)
//		IS interrupt-safe (1 cmd 0 resp)
//		NOT interrupt-safe (otherwise) (IRQ would try to handle the data we're waiting on)
MOSS_ND_INLINE STATUS _cmd_ctrl1_noresp_unsafe( uint8_t cmd1                             )
{
	return _w_cmd_unsafe( cmd1, TIMEOUTMS_CTRL_CMD_ACPT );
}
MOSS_ND_INLINE STATUS _cmd_ctrl1_resp1_unsafe ( uint8_t cmd1,              uint8_t* resp )
{
	STATUS ret = _w_cmd_unsafe( cmd1, TIMEOUTMS_CTRL_CMD_ACPT );
	if ( ret < 0 ) return ret;

	return _r_wait_data_unsafe( TIMEOUTMS_CTRL_CMD_EXEC, resp );
}
MOSS_ND_INLINE STATUS _cmd_ctrl2_noresp_unsafe( uint8_t cmd1,uint8_t cmd2                )
{
	STATUS ret = _w_cmd_unsafe( cmd1, TIMEOUTMS_CTRL_CMD_ACPT );
	if ( ret < 0 ) return ret;

	return _w_data_unsafe( cmd2, TIMEOUTMS_CTRL_CMD_ACPT ); // Yes, to data
}
MOSS_ND_INLINE STATUS _cmd_ctrl2_resp1_unsafe ( uint8_t cmd1,uint8_t cmd2, uint8_t* resp )
{
	STATUS ret = _w_cmd_unsafe( cmd1, TIMEOUTMS_CTRL_CMD_ACPT );
	if ( ret < 0 ) return ret;

	ret = _w_data_unsafe( cmd2, TIMEOUTMS_CTRL_CMD_ACPT ); // Yes, to data
	if ( ret < 0 ) return ret;

	return _r_wait_data_unsafe( TIMEOUTMS_CTRL_CMD_EXEC, resp );
}
//	Controller command read an internal RAM byte (config is byte 0)
//		NOT threadsafe
//		NOT interrupt-safe
MOSS_ND_INLINE static STATUS _cmd_ctrl_getRAMbyte_unsafe( uint8_t ind, uint8_t* val )
{
	__moss_assert( ind<=0x1F, "Invalid byte index %d!", (int)ind );
	return _cmd_ctrl1_resp1_unsafe( 0x20|ind, val );
}
MOSS_ND_INLINE static STATUS _cmd_ctrl_getconfig_unsafe( _Config* config )
{
	return _cmd_ctrl_getRAMbyte_unsafe( 0, &config->packed );
}
//	Controller command write an internal RAM byte (config is byte 0)
//		NOT threadsafe
//		IS interrupt-safe
MOSS_ND_INLINE static STATUS _cmd_ctrl_setRAMbyte_unsafe( uint8_t ind, uint8_t  val )
{
	__moss_assert( ind<=0x1F, "Invalid byte index %d!", (int)ind );

	return _cmd_ctrl2_noresp_unsafe( 0x60|ind, val );
}
MOSS_ND_INLINE static STATUS _cmd_ctrl_setconfig_unsafe( _Config  config )
{
	return _cmd_ctrl_setRAMbyte_unsafe( 0, config.packed );
}
//	Eat data
//		IS threadsafe
//		IS interrupt-safe
MOSS_INLINE void _eat_all_data(void)
{
	while ( _r_status().ctrl_buf_out_full ) (void)_r_data();
}
//	Device command
//		NOT threadsafe (other thread could write buffer, filling it so we lose our data)
//		NOT interrupt-safe (IRQ would try to handle the data we're waiting on)
MOSS_ND STATUS _cmd_devn_resp_unsafe(
	uint8_t cmd[],uint8_t numcmd, uint8_t ichan, uint8_t data[16],uint8_t* got, char const* cmdname
) {
	STATUS ret;
	uint8_t attempts = 0;
	RETRY:

	// Send command
	for ( uint8_t k=0; k<numcmd; ++k )
	{
		// If device on channel 2, tell controller next byte is for that device instead
		if ( ichan > 0 )
		{
			ret = _cmd_ctrl1_noresp_unsafe( 0xd4 );
			if ( ret < 0 ) return ret;
		}

		// Send command byte
		ret = _w_data_unsafe( cmd[k], TIMEOUTMS_DEV_CMD_ACPT );
		if ( ret < 0 ) return ret;
	}

	// Wait for any response byte(s)
	struct TinyRingBuffer tmpbuf = ds_trb_init();
	uint8_t count;
	for ( count=0; count<16; ++count )
	{
		uint8_t resp;
		ret = _r_wait_data_unsafe( TIMEOUTMS_DEV_CMD_EXEC, &resp );
		if ( ret == STATUS_x86_PS2_Timeout ) break;
		else if ( ret < 0 ) return ret;

		ds_trb_push_back( &tmpbuf, resp );
	}
	#if 0
	klogf( "LL got (%u): ", count );
	for ( uint8_t k=0; k<count; ++k ) klogf( "%02x%s", tmpbuf.data[k], k<count-1?", ":"\n" );
	#endif

	// Other devices could have pushed scanning codes in the meantime, so try to eat until we find
	// an ack (in PS/2, 0xfa) or resend (0xfe), which the command should have sent back
	for ( uint8_t k=0; k<count; ++k )
	{
		uint8_t val = ds_trb_peek_front( &tmpbuf );
		if ( val == 0xfe )
		{
			if ( attempts == 3 ) return STATUS_x86_PS2_DeviceCommError;
			++attempts;
			goto RETRY;
		}
		if ( val == 0xfa ) break;

		uint8_t discard;
		ds_trb_pop_front( &tmpbuf, &discard );
	}

	// Copy into output
	*got = 0;
	while ( !ds_trb_is_empty(&tmpbuf) )
	{
		ds_trb_pop_front( &tmpbuf, data+(*got)++ );
	}

	#if 1
	klogf( "PS/2: device %u %s output -> ", ichan, cmdname );
	for ( uint8_t k=0; k<*got; ++k ) klogf( "%02x%s", data[k], k<*got-1?", ":"\n" );
	#endif

	return STATUS_Success;
}
MOSS_ND_INLINE STATUS _cmd_dev1_resp_unsafe(
	uint8_t cmd1,              uint8_t ichan, uint8_t data[16],uint8_t* got, char const* cmdname
) {
	return _cmd_devn_resp_unsafe( &cmd1,1, ichan, data,got, cmdname );
}
MOSS_ND_INLINE STATUS _cmd_dev2_resp_unsafe(
	uint8_t cmd1,uint8_t cmd2, uint8_t ichan, uint8_t data[16],uint8_t* got, char const* cmdname
) {
	uint8_t cmd[2] = { cmd1, cmd2 };
	return _cmd_devn_resp_unsafe( cmd,2, ichan, data,got, cmdname );
}
//	Device command: reset
//		NOT threadsafe
//		NOT interrupt-safe
MOSS_ND STATUS _cmd_dev_reset_unsafe( uint8_t ichan )
{
	// All devices should respond to reset command; 0xfa=success, 0xfc=failure, none=not connected,
	// and possibly more bytes following.

	uint8_t data[16], got;
	STATUS ret = _cmd_dev1_resp_unsafe( 0xff, ichan, data,&got, "reset" );
	if ( ret < 0 ) return ret;

	if ( got<2 || data[0]!=0xfa ) return STATUS_x86_PS2_DeviceError;

	if ( data[1]==0xfc || data[1]==0xfd ) return STATUS_x86_PS2_TestFailedDevice;

	if ( data[1]!=0xaa ) return STATUS_x86_PS2_DeviceError;

	// May be more bytes (e.g. mouse sends 0x00)

	return ret;
}
//	Device command: disable scanning
MOSS_ND STATUS _cmd_dev_setscanning_unsafe( uint8_t ichan, bool enable_scanning )
{
	// 0xf5 disable scanning, 0xf4 enable scanning
	uint8_t data[16], got;
	STATUS ret = _cmd_dev1_resp_unsafe(
		0xf5-(int)enable_scanning, ichan, data,&got,
		enable_scanning ? "scanning enable" : "scanning disable"
	);
	if ( ret < 0 ) return ret;

	if ( got==0 || data[0]!=0xfa ) return STATUS_x86_PS2_DeviceError;

	return ret;
}
//	Device command: identify
MOSS_ND STATUS _cmd_dev_identify_unsafe( uint8_t ichan, struct x86_PS2_Device* dev )
{
	uint8_t data[16], got;
	STATUS ret = _cmd_dev1_resp_unsafe( 0xf2, ichan, data,&got, "identify" );
	if ( ret < 0 ) return ret;

	if ( got==0 || data[0]!=0xfa ) return STATUS_x86_PS2_DeviceError;

	// https://wiki.osdev.org/%228042%22_PS/2_Controller#Detecting_PS.2F2_Device_Types
	dev->type = DEVTYPE_x86_PS2_Unknown;
	switch ( got )
	{
		case 1: // Ancient AT keyboard
			dev->type = DEVTYPE_x86_PS2_Keyboard;
			break;
		case 2:
			switch ( data[1] )
			{
				case 0x00: // Standard PS/2 mouse
					dev->type = DEVTYPE_x86_PS2_Mouse;
					break;
				case 0x03: // Mouse with scroll wheel
					dev->type = DEVTYPE_x86_PS2_MouseScroll;
					break;
				case 0x04: // 5-button mouse
					dev->type = DEVTYPE_x86_PS2_MouseScrollButtons;
					break;
				default:
					break;
			}
			break;
		case 3:
			if      ( data[1] == 0xab )
			{
				switch ( data[2] )
				{
					case 0x83: case 0xc1: // MF2 keyboard
					case 0x84: // 'short' keyboard ("IBM ThinkPads, Spacesaver keyboards," etc.)
					case 0x85: // NCD N-97 keyboard, 122-Key Host Connect(ed) Keyboard
					case 0x86: // 122-key keyboards
					case 0x90: // Japanese "G" keyboards
					case 0x91: // Japanese "P" keyboards
					case 0x92: // Japanese "A" keyboards
						dev->type = DEVTYPE_x86_PS2_Keyboard;
						break;
					default:
						break;
				}
			}
			else if ( data[1]==0xac && data[2]==0xa1 ) // NCD Sun layout keyboard
			{
				dev->type = DEVTYPE_x86_PS2_Keyboard;
			}
			break;
		default:
			break;
	}

	return ret;
}



// Lower-level API: device-specific commands
MOSS_ND STATUS _cmd_devmouse_setrate_unsafe( uint8_t ichan, uint8_t rate_hz )
{
	uint8_t data[16], got;
	STATUS ret = _cmd_dev2_resp_unsafe( 0xf3,rate_hz, ichan, data,&got, "set resolution" );
	if ( ret < 0 ) return ret;

	if ( got==0 || data[0]!=0xfa ) ret=STATUS_x86_PS2_DeviceError;

	return ret;
}



// Public API

// IRQs
__attribute__(( no_caller_saved_registers ))
void x86_ps2_handle_irq( uint8_t irq )
{
	__moss_assert_impl( irq==1 || irq==12 ); // dev 0 or 1

	__moss_assert( g_x86_ps2ctrl.inited, "Got PS/2 IRQ before PS/2 controller inited!" );
	// probably, you enabled interrupts being initing

	struct x86_PS2_Device* dev = g_x86_ps2ctrl.devices + ((irq-1)&1);

	mutex_spin_acquire( &g_x86_ps2ctrl.lock );

	while ( _r_status().ctrl_buf_out_full )
	{
		uint8_t val = _r_data();
		ds_trb_trypush_back( &dev->recvbuf, val );
	}

	mutex_spin_release( &g_x86_ps2ctrl.lock );
}

// Initialize controller
MOSS_ND_INLINE STATUS _init_helper_unsafe( _Config* config )
{
	STATUS ret;
	uint8_t test;

	// See https://wiki.osdev.org/%228042%22_PS/2_Controller#Initialising_the_PS.2F2_Controller

	// Determine if the PS/2 controller exists (TODO)

	// Disable any devices to prevent them sending data while setting up, and then clear any data
	// they may have already sent.
	{
		ret = _cmd_ctrl1_noresp_unsafe( 0xad ); // disable channel 1
		if ( ret < 0 ) return ret;

		ret = _cmd_ctrl1_noresp_unsafe( 0xa7 ); // disable channel 2; ignored if only one channel
		if ( ret < 0 ) return ret;

		_eat_all_data();
	}

	// Disable interrupts and keyboard translation
	{
		// Note not interrupt-safe; fortunately interrupts are disabled
		ret = _cmd_ctrl_getconfig_unsafe( config );
		if ( ret < 0 ) return ret;
		//klogf( "A: got config %08b\n", config->packed );

		config->packed &= 0x34; // 0xb0011'0100, disable interrupts and translation

		ret = _cmd_ctrl_setconfig_unsafe( *config );
		if ( ret < 0 ) return ret;
		//klogf( "B: set config %08b\n", config->packed );
	}

	// Controller self-test
	// 0x55 is pass, 0xfc is fail.  If we somehow got anything ele, that would be a fail too
	{
		// Note not interrupt-safe; fortunately interrupts are disabled
		ret = _cmd_ctrl1_resp1_unsafe( 0xaa, &test );
		if ( ret < 0 ) return ret;

		if ( test != 0x55 ) return STATUS_x86_PS2_TestFailedCtrl;

		// some controllers reset themselves after, so fix that
		ret = _cmd_ctrl_setconfig_unsafe( *config );
		if ( ret < 0 ) return ret;
		//klogf( "C: set config %08b\n", config->packed );
	}

	// Get number of channels
	{
		g_x86_ps2ctrl.has_okay_chan[0] = 1;

		ret = _cmd_ctrl1_noresp_unsafe( 0xa8 ); // enable channel 2
		if ( ret < 0 ) return ret;

		ret = _cmd_ctrl_getconfig_unsafe( config );
		if ( ret < 0 ) return ret;
		//klogf( "D: got config %08b\n", config->packed );

		if ( config->disable_clock_chan2 == 0 )
		{
			g_x86_ps2ctrl.num_chans = 2;
			g_x86_ps2ctrl.has_okay_chan[1] = 1;

			ret = _cmd_ctrl1_noresp_unsafe( 0xa7 ); // disable channel 2 again; note changes config again
			if ( ret < 0 ) return ret;
		}
		else
		{
			g_x86_ps2ctrl.num_chans = 1;
			g_x86_ps2ctrl.has_okay_chan[1] = 0;
		}
	}

	// Interface tests
	{
		ret = _cmd_ctrl1_resp1_unsafe( 0xab, &test );
		if ( ret < 0 ) return ret;
		g_x86_ps2ctrl.has_okay_chan[0] = test==0x00;

		if ( g_x86_ps2ctrl.has_okay_chan[1] )
		{
			ret = _cmd_ctrl1_resp1_unsafe( 0xa9, &test );
			if ( ret < 0 ) return ret;
			g_x86_ps2ctrl.has_okay_chan[1] = test==0x00;
		}

		if ( !g_x86_ps2ctrl.has_okay_chan[0] && !g_x86_ps2ctrl.has_okay_chan[1] )
		{
			return STATUS_x86_PS2_TestFailedCtrlBothPorts;
		}
	}

	// Enable ports
	{
		if ( g_x86_ps2ctrl.has_okay_chan[0] )
		{
			ret = _cmd_ctrl1_noresp_unsafe( 0xae );
			if ( ret < 0 ) return ret;
		}
		if ( g_x86_ps2ctrl.has_okay_chan[1] )
		{
			ret = _cmd_ctrl1_noresp_unsafe( 0xa8 );
			if ( ret < 0 ) return ret;
		}
	}

	// Enable interrupts (though note calling before interrupts unmasked)
	{
		ret = _cmd_ctrl_getconfig_unsafe( config ); // need to get config again, changed
		if ( ret < 0 ) return ret;
		//klogf( "E: got config %08b\n", config->packed );

		if ( g_x86_ps2ctrl.has_okay_chan[0] ) config->interrupt_enable_chan1=1;
		if ( g_x86_ps2ctrl.has_okay_chan[1] ) config->interrupt_enable_chan2=1;

		ret = _cmd_ctrl_setconfig_unsafe( *config );
		if ( ret < 0 ) return ret;
		//klogf( "F: set config %08b\n", config->packed );
	}

	// Disable scanning for now
	{
		// Keyboard usually off, mouse usually on, set it consistent.  Failure is okay.
		if ( g_x86_ps2ctrl.has_okay_chan[0] ) (void)_cmd_dev_setscanning_unsafe( 0, false );
		if ( g_x86_ps2ctrl.has_okay_chan[0] ) (void)_cmd_dev_setscanning_unsafe( 1, false );
	}

	return ret;
}
MOSS_ND STATUS x86_ps2_init(void)
{
	// Note: `g_x86_ps2ctrl` guaranteed to be initialized to zero, since global

	STATUS ret;
	mutex_spin_acquire( &g_x86_ps2ctrl.lock );
	{
		klogf("PS/2: setting up . . .\n");
		__moss_assert( !x86_core_interrupts_are_enabled(),
			"Must be called with interrupts disabled!"
		);

		__moss_assert( !g_x86_ps2ctrl.inited, "PS/2: already set up!" );
		// maybe in another thread?

		_Config config;
		ret = _init_helper_unsafe( &config );
		if ( ret < 0 ) goto END_CS;

		klogf( "PS/2: controller setup successful!  < chans=(%d,%d), config=%08b, status=%08b >\n",
			(int)g_x86_ps2ctrl.has_okay_chan[0], (int)g_x86_ps2ctrl.has_okay_chan[1],
			(int)config.packed, (int)_r_status().packed
		);
		MOSS_DEBUG_ONLY( g_x86_ps2ctrl.inited = true; )
	}
	END_CS:
	mutex_spin_release( &g_x86_ps2ctrl.lock );

	return ret;
}

// (Re)-detect device on channel
MOSS_ND_INLINE STATUS _redetect_helper_unsafe( uint8_t ichan )
{
	// Start with a clean slate (note sets device type to `DEVTYPE_x86_PS2_None`)
	struct x86_PS2_Device* dev       = g_x86_ps2ctrl.devices +    ichan ;
	struct x86_PS2_Device* dev_other = g_x86_ps2ctrl.devices + (1-ichan);
	memset( dev, 0x00, sizeof(struct x86_PS2_Device) );
	__moss_assert_impl( dev->type == DEVTYPE_x86_PS2_None );

	// Reset / self-test
	STATUS ret = _cmd_dev_reset_unsafe( ichan );
	if ( ret < 0 ) return ret;

	// Disable scanning of this device (shouldn't be on)
	ret = _cmd_dev_setscanning_unsafe( ichan, false );
	if ( ret < 0 ) return ret;

	// Disable scanning on the other device (for now); could screw up identify
	bool toggle_other = dev_other->type!=DEVTYPE_x86_PS2_None && dev_other->scanning_enabled;
	if ( toggle_other )
	{
		ret = _cmd_dev_setscanning_unsafe( 1-ichan, false );
		if ( ret < 0 ) return ret;
	}

	// Detect device type
	ret = _cmd_dev_identify_unsafe( ichan, dev );
	if ( ret < 0 ) goto CLEANUP;

	#if 1
	if ( dev->type == DEVTYPE_x86_PS2_Mouse )
	{
		// See if we have a scrollwheel https://wiki.osdev.org/PS/2_Mouse#Mouse_Extensions

		ret = _cmd_devmouse_setrate_unsafe( ichan, 200 );
		if ( ret < 0 ) goto CLEANUP_MOUSE_UPGRADE;
		ret = _cmd_devmouse_setrate_unsafe( ichan, 100 );
		if ( ret < 0 ) goto CLEANUP_MOUSE_UPGRADE;
		ret = _cmd_devmouse_setrate_unsafe( ichan,  80 );
		if ( ret < 0 ) goto CLEANUP_MOUSE_UPGRADE;

		ret = _cmd_dev_identify_unsafe( ichan, dev );
		if ( ret < 0 ) goto CLEANUP_MOUSE_UPGRADE;

		#if 1
		if ( dev->type == DEVTYPE_x86_PS2_MouseScroll )
		{
			// See if we have a 5-button mouse

			ret = _cmd_devmouse_setrate_unsafe( ichan, 200 );
			if ( ret < 0 ) goto CLEANUP_MOUSE_UPGRADE;
			ret = _cmd_devmouse_setrate_unsafe( ichan, 200 );
			if ( ret < 0 ) goto CLEANUP_MOUSE_UPGRADE;
			ret = _cmd_devmouse_setrate_unsafe( ichan,  80 );
			if ( ret < 0 ) goto CLEANUP_MOUSE_UPGRADE;

			ret = _cmd_dev_identify_unsafe( ichan, dev );
			if ( ret < 0 ) goto CLEANUP_MOUSE_UPGRADE;
		}
		#endif

		// Set it back to default
		CLEANUP_MOUSE_UPGRADE:
		ret = _cmd_devmouse_setrate_unsafe( ichan, 100 );
		if ( ret < 0 ) goto CLEANUP;
	}
	#endif

	CLEANUP:
	// Re-enable scanning on the other device if we turned it off
	if ( toggle_other )
	{
		ret = _cmd_dev_setscanning_unsafe( 1-ichan, true );
		if ( ret < 0 ) return ret;
	}

	klogf( "PS/2: device detected, type %d\n", dev->type );
	switch ( dev->type )
	{
		case DEVTYPE_x86_PS2_Keyboard          :
			dev->dev_id = inpdevs_add_kb();
			break;
		case DEVTYPE_x86_PS2_Mouse             : [[fallthrough]];
		case DEVTYPE_x86_PS2_MouseScroll       : [[fallthrough]];
		case DEVTYPE_x86_PS2_MouseScrollButtons:
			dev->dev_id = inpdevs_add_mouse();
			break;
		__moss_switch_default_unnecessary;
	}
	return ret;
}
MOSS_ND STATUS x86_ps2_redetect_device( uint8_t ichan )
{
	STATUS ret;
	x86flags old_flags = x86_core_interrupts_push_and_disable();
	mutex_spin_acquire( &g_x86_ps2ctrl.lock );
	{
		klogf("PS/2: (re)detecting any device on channel %u . . .\n", ichan );
		__moss_assert( g_x86_ps2ctrl.inited, "Must initialize controller first!" );

		if ( !g_x86_ps2ctrl.has_okay_chan[ichan] )
		{
			ret = STATUS_x86_PS2_BadChannel;
			goto END_CS;
		}

		ret = _redetect_helper_unsafe( ichan );
		if ( ret < 0 ) goto END_CS;

		klogf( "PS/2: device setup complete!\n" );
	}
	END_CS:
	mutex_spin_release( &g_x86_ps2ctrl.lock );
	x86_core_interrupts_pop( old_flags );

	return ret;
}
// (Re-)detect devices
MOSS_ND STATUS x86_ps2_redetect_devices(void)
{
	__moss_assert( g_x86_ps2ctrl.inited, "Must initialize controller first!" );

	STATUS ret = STATUS_Success;

	if ( g_x86_ps2ctrl.has_okay_chan[0] )
	{
		ret = x86_ps2_redetect_device( 0 );
		if ( ret < 0 ) goto DONE;
	}
	if ( g_x86_ps2ctrl.has_okay_chan[1] )
	{
		ret = x86_ps2_redetect_device( 1 );
		if ( ret < 0 ) goto DONE;
	}

	DONE:
	return ret;
}

MOSS_ND STATUS x86_ps2_set_scanning( uint8_t ichan, bool enable_scanning )
{
	STATUS ret;
	x86flags old_flags = x86_core_interrupts_push_and_disable();
	mutex_spin_acquire( &g_x86_ps2ctrl.lock );
	{
		__moss_assert( g_x86_ps2ctrl.inited, "Must initialize controller first!" );
		ret = _cmd_dev_setscanning_unsafe( ichan, enable_scanning );
	}
	mutex_spin_release( &g_x86_ps2ctrl.lock );
	x86_core_interrupts_pop( old_flags );

	return ret;
}

static void _handle_data_helper_unsafe( uint8_t ichan )
{
	struct x86_PS2_Device* dev = g_x86_ps2ctrl.devices + ichan;
	uint8_t count = ds_trb_count( &dev->recvbuf );

	bool parsed;
	if ( count == 0 ) { parsed=true; goto SET_LAST_PARSE_TIME; }

	#if 0
	klogf( "PS/2: device %u buffer -> [", ichan );
	for ( uint8_t k=0; k<count; ++k ) klogf(
		"%02x%s", ds_trb_get(&dev->recvbuf,k), k<count-1?",":""
	);
	klogf( "](%u)\n", count );
	#endif

	// Try to parse data packet
	parsed = false;
	switch ( dev->type )
	{
		default: [[fallthrough]];
		case DEVTYPE_x86_PS2_None: return;

		case DEVTYPE_x86_PS2_Keyboard:
		{
			struct InpDev_Keyboard* kb = inpdev_kb_get( dev->dev_id );

			while ( count > 0 )
			{
				//ds_trb_clear( &dev->recvbuf );

				uint8_t buf[8];
				memset( buf, 0x00, 8 );
				for ( uint8_t k=0; k<MIN(count,8); ++k ) buf[k]=ds_trb_get(&dev->recvbuf,k);

				unsigned num = kb->inpdev_kb_parse_ps2_sc_seti( dev->dev_id, buf );
				kassert( num<=count && num<=8, "Parsed too much (%u out of %u, max 8)", num,count );
				if ( num == 0 ) break;

				#if 1
				klogs( " [" );
				for ( uint8_t k=0; k<count; ++k ) klogf(
					"%02x%s", ds_trb_get(&dev->recvbuf,k), k<count-1?",":""
				);
				klogc( ']' );
				klogf( " (ate %u)\n", num );
				#endif

				for ( unsigned k=0; k<num; ++k ) ds_trb_pop_front(&dev->recvbuf,buf);
				kassert( count-num==ds_trb_count(&dev->recvbuf),
					"Popped %u from list of length %u; got unexpected size %u",
					num, count, ds_trb_count(&dev->recvbuf)
				);

				parsed = true;
				count -= num;
			}
			break;
		}

		// https://wiki.osdev.org/PS/2_Mouse
		// https://wiki.osdev.org/Mouse_Input#Formats_of_Optional_4th_Packet_Byte
		case DEVTYPE_x86_PS2_Mouse             : [[fallthrough]];
		case DEVTYPE_x86_PS2_MouseScroll       : [[fallthrough]];
		case DEVTYPE_x86_PS2_MouseScrollButtons:
		{
			uint8_t packet_sz = dev->type == DEVTYPE_x86_PS2_Mouse ? 3 : 4;
			struct InpDev_Mouse* mouse = inpdev_mouse_get( dev->dev_id );

			while ( count >= packet_sz )
			{
				uint8_t byte0, byte1, byte2, byte3;
				ds_trb_pop_front( &dev->recvbuf, &byte0 );
				ds_trb_pop_front( &dev->recvbuf, &byte1 );
				ds_trb_pop_front( &dev->recvbuf, &byte2 );
				if ( packet_sz == 4 ) ds_trb_pop_front( &dev->recvbuf, &byte3 );

				vec2i rel =
				{
					.x = (int32_t)byte1 - ((byte0<<4)&0x0100), // shift sign bit into position
					.y = (int32_t)byte2 - ((byte0<<3)&0x0100)  // shift sign bit into position
				};
				if ( rel.x!=0 || rel.y!=0 )
				{
					rel.y = -rel.y;
					inpdev_mouse_moved( dev->dev_id, rel );
				}
				//klogf( "[%d,%d] -> [%d,%d]\n", rel.x,rel.y, mouse->rel.x,mouse->rel.y );

				bool btns[5];
				int rel_scroll;

				btns[0] = (bool)( byte0 & 0x01 );
				btns[1] = (bool)( byte0 & 0x04 ); // note different order
				btns[2] = (bool)( byte0 & 0x02 );

				if ( dev->type == DEVTYPE_x86_PS2_MouseScrollButtons )
				{
					btns[3] = (bool)( byte3 & 0x10 );
					btns[4] = (bool)( byte3 & 0x20 );
					rel_scroll = (int32_t)((uint32_t)byte3<<28) >> 28;
				}
				else
				{
					btns[3] = false;
					btns[4] = false;
					if ( dev->type == DEVTYPE_x86_PS2_Mouse )
					{
						rel_scroll = 0;
					}
					else
					{
						rel_scroll = (int)(int8_t)byte3;
					}
				}

				if ( rel_scroll != 0 )
				{
					inpdev_mouse_scroll( dev->dev_id, rel_scroll );
				}
				for ( unsigned k=0; k<5; ++k )
				{
					if      ( !mouse->buttons[k] &&  btns[k]  )
					{
						inpdevs_got_btn_press  ( dev->dev_id, INPDEV_BTN_MouseLeft+k );
					}
					else if (  mouse->buttons[k] && !btns[k]  )
					{
						inpdevs_got_btn_release( dev->dev_id, INPDEV_BTN_MouseLeft+k );
					}
				}

				#if 0
				klogf( "PS/2: device %u (id %u) mouse: [%+d,%+d]->[%d,%d], btn=<%d,%d,%d",
					ichan, dev->dev_id,
					mouse->rel.x, mouse->rel.y,
					mouse->pos.x, mouse->pos.y,
					mouse->buttons[0], mouse->buttons[1], mouse->buttons[2]
				);
				if ( dev->type == DEVTYPE_x86_PS2_MouseScrollButtons ) klogf( ",%d,%d",
					mouse->buttons[3], mouse->buttons[4]
				);
				klogc('>');
				if ( dev->type != DEVTYPE_x86_PS2_Mouse ) klogf( ", scroll=%d",
					mouse->rel_scroll
				);
				klogc('\n');
				#endif

				parsed = true;
				count -= packet_sz;
			}
			break;
		}
	}

	SET_LAST_PARSE_TIME:
	{
		Cycles timestamp_now = x86_rdtsc_nofences();
		if ( parsed )
		{
			dev->timestamp_last_parse = timestamp_now;
		}
		else
		{
			Cycles cycles_since = timestamp_now - dev->timestamp_last_parse;
			if ( cycles_since > kernel_conv_delayms_to_cycles(TIMEOUTMS_DEV_PACKET) )
			{
				klogf( "It has been %" PRIu64 " cycles since last packet, clearing!\n",
					cycles_since
				);
				dev->timestamp_last_parse = timestamp_now;
				ds_trb_clear( &dev->recvbuf );
			}
		}
	}
}
void x86_ps2_handle_data(void)
{
	x86flags old_flags = x86_core_interrupts_push_and_disable();
	mutex_spin_acquire( &g_x86_ps2ctrl.lock );
	{
		__moss_assert( g_x86_ps2ctrl.inited, "Must initialize controller first!" );
		_handle_data_helper_unsafe(0);
		_handle_data_helper_unsafe(1);
	}
	mutex_spin_release( &g_x86_ps2ctrl.lock );
	x86_core_interrupts_pop( old_flags );
}



// Public API: device-specific commands

MOSS_ND STATUS x86_ps2_devmouse_setres( uint8_t ichan, uint8_t res_0through3 )
{
	STATUS ret;
	x86flags old_flags = x86_core_interrupts_push_and_disable();
	mutex_spin_acquire( &g_x86_ps2ctrl.lock );
	{
		__moss_assert( g_x86_ps2ctrl.inited, "Must initialize controller first!" );

		uint8_t data[16], got;
		ret = _cmd_dev2_resp_unsafe( 0xe8,res_0through3, ichan, data,&got, "set resolution" );
		if ( ret < 0 ) return ret;

		if ( got==0 || data[0]!=0xfa ) ret=STATUS_x86_PS2_DeviceError;
	}
	mutex_spin_release( &g_x86_ps2ctrl.lock );
	x86_core_interrupts_pop( old_flags );

	return ret;
}
MOSS_ND STATUS x86_ps2_devmouse_setrate( uint8_t ichan, uint8_t rate_hz )
{
	STATUS ret;
	x86flags old_flags = x86_core_interrupts_push_and_disable();
	mutex_spin_acquire( &g_x86_ps2ctrl.lock );
	{
		__moss_assert( g_x86_ps2ctrl.inited, "Must initialize controller first!" );

		ret = _cmd_devmouse_setrate_unsafe( ichan, rate_hz );
	}
	mutex_spin_release( &g_x86_ps2ctrl.lock );
	x86_core_interrupts_pop( old_flags );

	return ret;
}
