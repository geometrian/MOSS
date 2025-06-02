#include "stdint.h"



// ARGB
#define ORANGE 0xffffa500
#define CYAN   0xff00ffff
#define RED    0xffff0000
#define GREEN  0xff00ff00
#define BLUE   0xff0000ff
#define GRAY   0xff888888
#define WHITE  0xffffffff
#define BLACK  0xff000000

struct GRAPHICS_COLOR_PIXEL
{
	uint8_t   Blue;
	uint8_t   Green;
	uint8_t   Red;
	uint8_t   Alpha;
};

// From the UEFI PDF Page 170
struct EFI_MEMORY_DESCRIPTOR
{
	uint32_t  type;
	uint64_t* PhysicalAddress;
	uint64_t* VirtualAddress;
	uint64_t  NumberOfPages;
	uint64_t  Attributes;
};

struct BLOCKINFO
{
	uint64_t               BaseAddress;
	uint64_t               BufferSize;
	uint32_t               ScreenWidth;
	uint32_t               ScreenHeight;
	uint32_t               PixelsPerScanLine;
	uint64_t               LoaderFileSize;
	EFI_MEMORY_DESCRIPTOR* MMap;
	uint64_t               MMapSize;
	uint64_t               MMapDescriptorSize;
	uint64_t*              rsdp;
} __attribute__((__packed__));

BLOCKINFO* biStruct __attribute__((section (".text")));

void CreateBufferFilledBox( uint32_t xPos,uint32_t yPos, uint32_t w,uint32_t h, uint32_t c );

void main(BLOCKINFO* bi)
{
	biStruct = bi;
	biStruct->BaseAddress = bi->BaseAddress; // - 128);   // Somehow this fixes it.
	
	CreateBufferFilledBox(0, 0, 150, 150, ORANGE);
	
	while(1){__asm__ ("hlt");}
}

void CreateBufferFilledBox(uint32_t xPos, uint32_t yPos, uint32_t w, uint32_t h, uint32_t c)
{
	uint32_t x;
	uint32_t y      = yPos;
	uint32_t width  = xPos + w;
	uint32_t height = yPos + h;

	for     (y = yPos; y <= height; y++)
	{
		for (x = xPos; x <= width; x++)
		{
			*(x + (y * biStruct->PixelsPerScanLine) + (uint32_t*)(biStruct->BaseAddress)) = c;
		}
	}
}

