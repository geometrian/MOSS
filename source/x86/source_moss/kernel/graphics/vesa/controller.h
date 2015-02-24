#pragma once

#include "../../../includes.h"


namespace MOSS { namespace Graphics {
	class Color;
namespace VESA {


//http://www.delorie.com/djgpp/doc/ug/graphics/vesa.html
//http://wiki.osdev.org/Getting_VBE_Mode_Info

//The VESA callbacks require addresses below 1MiB or else they cannot be accessed through far pointers.
//In particular, when the VESA info. or a mode's info. is retrieved, the pointer 0x500 is used directly.
//This address should be free for use after we get all our information from the bootloader.  After using
//these pointers, the data is immediately copied to the objects themselves.
//See http://wiki.osdev.org/Memory_Map_%28x86%29


class Framebuffer;

class Mode;

class Controller {
	public:
		int width;
		int height;

		int numof_modes;
		Mode** modes;
		Mode* current_mode;

	private:
		Framebuffer* framebuffers[2];
		int latest_complete_framebuffer;
		int framebuffer_writing;
	public:
		Framebuffer* current_framebuffer;

	private:
		class VESA_INFO final { public:
			uint8_t  VESASignature[4]; //set to "VBE2", returns "VESA"
			uint16_t VESAVersion;
			uint32_t OEMStringPtr;
			uint8_t  Capabilities[4];
			uint32_t VideoModePtr;
			uint16_t TotalMemory; //# of 64KB blocks
			uint16_t OemSoftwareRev;
			uint32_t OemVendorNamePtr;
			uint32_t OemProductNamePtr;
			uint32_t OemProductRevPtr;
			uint8_t  Reserved[222];
			uint8_t  OemData[256];
		} __attribute__((packed));
		VESA_INFO info;

	public:
		Controller(void);
		~Controller(void);

		Mode* get_mode_closest(int w,int h, int bpp);

		bool set_mode(Mode* mode);

		void frame_start(void);
		void frame_end(void);
		void frame_flip(void);
};


}}}
