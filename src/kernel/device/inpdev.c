#include "inpdev.h"



static_assert( sizeof(struct InpDev_Base) == MAX(
	sizeof(struct InpDev_Keyboard), sizeof(struct InpDev_Mouse)
));



struct InpDevs_Controller g_inpdev_ctrl;

void inpdevs_init(void)
{
	//memset( &g_inpdev_ctrl, 0x00, sizeof(struct InpDevs_Controller) );
	g_inpdev_ctrl.devices_count = 0;
}

MOSS_ND DEVID inpdevs_add_kb   (void)
{
	DEVID id = g_inpdev_ctrl.devices_count++;
	__moss_assert( id<2, "Overflow!" );
	struct InpDev_Keyboard* kb = (struct InpDev_Keyboard*)( g_inpdev_ctrl.devices + id );

	memset( kb, 0x00, sizeof(struct InpDev_Keyboard) );
	kb->type = DEVTYPE_InputKeyboard;
	kb->is_ansi_like = true;
	kb->inpdev_kb_parse_ps2_sc_seti = &inpdev_kb_parse_ps2_sc_set2;

	return id;
}
MOSS_ND DEVID inpdevs_add_mouse(void)
{
	DEVID id = g_inpdev_ctrl.devices_count++;
	__moss_assert( id<2, "Overflow!" );
	struct InpDev_Mouse* mouse = (struct InpDev_Mouse*)( g_inpdev_ctrl.devices + id );

	memset( mouse, 0x00, sizeof(struct InpDev_Mouse) );
	mouse->type = DEVTYPE_InputMouse;

	return id;
}

void inpdevs_got_btn_press            ( DEVID dev_id, INPDEV_BTN inpdev_btn )
{
	__moss_assert( dev_id<g_inpdev_ctrl.devices_count, "Invalid device %u!", dev_id );
	switch ( g_inpdev_ctrl.devices[dev_id].type )
	{
		case DEVTYPE_InputKeyboard:
			klogf( "Key-press \"%s\"\n", inpdev_btn_to_str(inpdev_btn) );
			inpdev_kb_set_pressed( dev_id, inpdev_btn );
			break;

		case DEVTYPE_InputMouse:
		{
			klogf( "Mouse-press \"%s\"\n", inpdev_btn_to_str(inpdev_btn) );
			__moss_assert( inpdev_btn>=INPDEV_BTN_MouseLeft && inpdev_btn<=INPDEV_BTN_MouseButton5,
				"Invalid mouse button %u!", inpdev_btn
			);
			struct InpDev_Mouse* mouse = inpdev_mouse_get( dev_id );
			mouse->buttons[ inpdev_btn - INPDEV_BTN_MouseLeft ] = true;
			break;
		}

		__moss_switch_default_unnecessary;
	}
}
void inpdevs_got_btn_release          ( DEVID dev_id, INPDEV_BTN inpdev_btn )
{
	__moss_assert( dev_id<g_inpdev_ctrl.devices_count, "Invalid device %u!", dev_id );
	switch ( g_inpdev_ctrl.devices[dev_id].type )
	{
		case DEVTYPE_InputKeyboard:
			klogf( "Key-release \"%s\"\n", inpdev_btn_to_str(inpdev_btn) );
			inpdev_kb_set_released( dev_id, inpdev_btn );
			break;

		case DEVTYPE_InputMouse:
		{
			klogf( "Mouse-release \"%s\"\n", inpdev_btn_to_str(inpdev_btn) );
			__moss_assert( inpdev_btn>=INPDEV_BTN_MouseLeft && inpdev_btn<=INPDEV_BTN_MouseButton5,
				"Invalid mouse button %u!", inpdev_btn
			);
			struct InpDev_Mouse* mouse = inpdev_mouse_get( dev_id );
			mouse->buttons[ inpdev_btn - INPDEV_BTN_MouseLeft ] = false;
			break;
		}

		__moss_switch_default_unnecessary;
	}
}
void inpdevs_got_btn_press_and_release( DEVID dev_id, INPDEV_BTN inpdev_btn )
{
	__moss_assert( dev_id<g_inpdev_ctrl.devices_count, "Invalid device %u!", dev_id );
	switch ( g_inpdev_ctrl.devices[dev_id].type )
	{
		case DEVTYPE_InputKeyboard:
			klogf( "Key-press-and-release \"%s\"\n", inpdev_btn_to_str(inpdev_btn) );
			inpdev_kb_set_released( dev_id, inpdev_btn );
			break;

		case DEVTYPE_InputMouse:
		{
			klogf( "Mouse-press-and-release \"%s\"\n", inpdev_btn_to_str(inpdev_btn) );
			__moss_assert( inpdev_btn>=INPDEV_BTN_MouseLeft && inpdev_btn<=INPDEV_BTN_MouseButton5,
				"Invalid mouse button %u!", inpdev_btn
			);
			struct InpDev_Mouse* mouse = inpdev_mouse_get( dev_id );
			mouse->buttons[ inpdev_btn - INPDEV_BTN_MouseLeft ] = false;
			break;
		}

		__moss_switch_default_unnecessary;
	}
}
