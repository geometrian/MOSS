#pragma once
#include "../../includes.h"


namespace MOSS {
	namespace Terminal {
		class TextModeTerminal;
	}
namespace Graphics {
	class Color;
namespace VESA {


//http://www.delorie.com/djgpp/doc/ug/graphics/vesa.html
//http://wiki.osdev.org/Getting_VBE_Mode_Info

//The VESA callbacks require addresses below 1MiB or else they cannot be accessed through far pointers.
//In particular, when the VESA info. or a mode's info. is retrieved, the pointer 0x500 is used directly.
//This address should be free for use after we get all our information from the bootloader.  After using
//these pointers, the data is immediately copied to the objects themselves.
//See http://wiki.osdev.org/Memory_Map_%28x86%29

class Mode {
	friend class Controller;
	private:
		bool valid;

		class MODE_INFO { public:
			unsigned short ModeAttributes;
			unsigned char  WinAAttributes;
			unsigned char  WinBAttributes;
			unsigned short WinGranularity;
			unsigned short WinSize;
			unsigned short WinASegment;
			unsigned short WinBSegment;
			unsigned long  WinFuncPtr;
			unsigned short BytesPerScanLine;
			unsigned short XResolution;
			unsigned short YResolution;
			unsigned char  XCharSize;
			unsigned char  YCharSize;
			unsigned char  NumberOfPlanes;
			unsigned char  BitsPerPixel;
			unsigned char  NumberOfBanks;
			unsigned char  MemoryModel;
			unsigned char  BankSize;
			unsigned char  NumberOfImagePages;
			unsigned char  Reserved_page;
			unsigned char  RedMaskSize;
			unsigned char  RedMaskPos;
			unsigned char  GreenMaskSize;
			unsigned char  GreenMaskPos;
			unsigned char  BlueMaskSize;
			unsigned char  BlueMaskPos;
			unsigned char  ReservedMaskSize;
			unsigned char  ReservedMaskPos;
			unsigned char  DirectColorModeInfo;
			unsigned long  PhysBasePtr;
			unsigned long  OffScreenMemOffset;
			unsigned short OffScreenMemSize;
			unsigned char  Reserved[206];
		} __attribute__((packed));
		MODE_INFO info;
		const uint16_t index;

	public:
		Mode(uint16_t index);
		~Mode(void);

		void print(Terminal::TextModeTerminal* terminal) const;
};

class Controller {
	public:
		int width;
		int height;

		int numof_modes;
		Mode** modes;
		Mode* current_mode;

	private:
		class VESA_INFO { public:
			unsigned char  VESASignature[4]; //set to "VBE2", returns "VESA"
			unsigned short VESAVersion;
			unsigned long  OEMStringPtr;
			unsigned char  Capabilities[4];
			unsigned long  VideoModePtr;
			unsigned short TotalMemory; //# of 64KB blocks
			unsigned short OemSoftwareRev;
			unsigned long  OemVendorNamePtr;
			unsigned long  OemProductNamePtr;
			unsigned long  OemProductRevPtr;
			unsigned char  Reserved[222];
			unsigned char  OemData[256];
		} __attribute__((packed));
		VESA_INFO info;

	public:
		Controller(void);
		~Controller(void);

		Mode* get_mode_closest(int w,int h, int bpp);

		bool set_mode(Mode* mode);

		void fill(const Color& color);
		void draw_text(int x,int y, char text, const Color& color);
		void draw_text(int x,int y, char text, const Color& color,const Color& background);
		void draw_text(int x,int y, const char* text, const Color& color);
		void draw_text(int x,int y, const char* text, const Color& color,const Color& background);
		void set_pixel(int x,int y, const Color& color);
		void blend_pixel(int x,int y, const Color& color);
};


}}}