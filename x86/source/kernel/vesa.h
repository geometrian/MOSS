#pragma once
#include "../includes.h"


namespace MOSS { namespace VESA {


//http://www.delorie.com/djgpp/doc/ug/graphics/vesa.html
//http://wiki.osdev.org/Getting_VBE_Mode_Info

typedef struct __attribute__((packed)) {
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
} VESA_INFO;

typedef struct __attribute__((packed)) {
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
} MODE_INFO;

bool get_vesa_info(VESA_INFO* vesa_info);
int get_mode_info(MODE_INFO* mode_info, uint16_t mode);

int find_vesa_mode(int w, int h, int depth);

bool set_vesa_mode(int w, int h, int depth);

//void set_vesa_bank(int bank_number);
void set_vesa_pixel(int x,int y, unsigned int rgba);
void set_vesa_pixel(int x,int y, unsigned char r,unsigned char g,unsigned char b,unsigned char a=255);


}}