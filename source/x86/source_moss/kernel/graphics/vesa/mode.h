#pragma once

#include "../../../includes.h"


namespace MOSS {
	namespace Terminal {
		class TextModeTerminal;
	}
namespace Graphics { namespace VESA {


class Mode final {
	public:
		bool valid;

		class MODE_INFO final { public:
			uint16_t ModeAttributes;
			uint8_t  WinAAttributes;
			uint8_t  WinBAttributes;
			uint16_t WinGranularity;
			uint16_t WinSize;
			uint16_t WinASegment;
			uint16_t WinBSegment;
			uint32_t WinFuncPtr;
			uint16_t BytesPerScanLine;
			uint16_t XResolution;
			uint16_t YResolution;
			uint8_t  XCharSize;
			uint8_t  YCharSize;
			uint8_t  NumberOfPlanes;
			uint8_t  BitsPerPixel;
			uint8_t  NumberOfBanks;
			uint8_t  MemoryModel;
			uint8_t  BankSize;
			uint8_t  NumberOfImagePages;
			uint8_t  Reserved_page;
			uint8_t  RedMaskSize;
			uint8_t  RedMaskPos;
			uint8_t  GreenMaskSize;
			uint8_t  GreenMaskPos;
			uint8_t  BlueMaskSize;
			uint8_t  BlueMaskPos;
			uint8_t  ReservedMaskSize;
			uint8_t  ReservedMaskPos;
			uint8_t  DirectColorModeInfo;
			uint32_t PhysBasePtr;
			uint32_t OffScreenMemOffset;
			uint16_t OffScreenMemSize;
			uint8_t  Reserved[206];
		} __attribute__((packed));
		MODE_INFO info;
		uint16_t const index;

	public:
		explicit Mode(uint16_t index);
		~Mode(void);

		void get_printable(char* buffer) const;
		void print(void) const;
};


}}}
