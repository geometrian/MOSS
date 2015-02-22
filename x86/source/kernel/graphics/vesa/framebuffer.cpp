#include "framebuffer.h"

#include "../../../mossc/cstdio"
#include "../../../mossc/cstdlib"
#include "../../../mossc/cstring"

#include "../color.h"
#include "../font.h"

#include "mode.h"


namespace MOSS { namespace Graphics { namespace PixelUtil {


//See http://wiki.osdev.org/Bochs_Graphics_Adaptor#Memory_layout_of_video_modes
class _Pixel15Channels { public:
	uint32_t      b : 5;
	uint32_t      g : 5;
	uint32_t      r : 5;
	uint32_t unused : 1;
} __attribute__((packed));
class _Pixel16Channels { public:
	uint32_t b : 5;
	uint32_t g : 6;
	uint32_t r : 5;
} __attribute__((packed));
#define CHANNEL_SCALE(CHANNEL, SRC,DST, MAX_SRC_SIZE,MAX_DST_SIZE)\
	DST.CHANNEL = (uint32_t)( MAX_DST_SIZE##.0f*(float)(SRC.CHANNEL)/MAX_SRC_SIZE##.0f )
class Pixel15 { public:
	union {
		_Pixel15Channels channels;
		uint16_t color;
	};
	Pixel15(const Color& color) {  CHANNEL_SCALE(r, color,channels, 255,31);  CHANNEL_SCALE(g, color,channels, 255,31);  CHANNEL_SCALE(b, color,channels, 255,31);  channels.unused=0u;  }
	Pixel15(const void* addr) : color(*( (uint16_t*)(addr) )) {}
	Color get(void) const {
		Color result;
		CHANNEL_SCALE(r, channels,result, 31,255);  CHANNEL_SCALE(g, channels,result, 31,255);  CHANNEL_SCALE(b, channels,result, 31,255);
		result.a = 255u;
		return result;
	}
	void set(void* addr) const { *((uint16_t*)(addr))=color; }
};
class Pixel16 { public:
	union {
		_Pixel16Channels channels;
		uint16_t color;
	};
	Pixel16(const Color& color) {  CHANNEL_SCALE(r, color,channels, 255,31);  CHANNEL_SCALE(g, color,channels, 255,63);  CHANNEL_SCALE(b, color,channels, 255,31);  }
	Pixel16(const void* addr) : color(*( (uint16_t*)(addr) )) {}
	Color get(void) const {
		Color result;
		CHANNEL_SCALE(r, channels,result, 31,255);  CHANNEL_SCALE(g, channels,result, 63,255);  CHANNEL_SCALE(b, channels,result, 31,255);
		result.a = 255u;
		return result;
	}
	void set(void* addr) const { *((uint16_t*)(addr))=color; }
};
class Pixel24 { public:
	struct {
		uint8_t b;
		uint8_t g;
		uint8_t r;
	} channels;
	Pixel24(const Color& color) { channels.r=color.r; channels.g=color.g; channels.b=color.b; }
	Pixel24(const void* addr) { const unsigned char* addr2=(const unsigned char*)(addr); channels.r=addr2[2]; channels.g=addr2[1]; channels.b=addr2[0]; }
	Color get(void) const { return Color(channels.r,channels.g,channels.b,255u); }
	void set(void* addr) const { unsigned char* addr2=(unsigned char*)(addr); addr2[2]=channels.r; addr2[1]=channels.g; addr2[0]=channels.b; }
} __attribute__((packed));
class Pixel32 { public:
	union {
		struct {
			uint8_t b;
			uint8_t g;
			uint8_t r;
			uint8_t a;
		} channels;
		uint32_t bgra;
	};
	Pixel32(const Color& color) { channels.r=color.r; channels.g=color.g; channels.b=color.b; channels.a=color.a; }
	Pixel32(const void* addr) : bgra(*( (uint32_t*)(addr) )) {}
	Color get(void) const { return Color(channels.r,channels.g,channels.b,channels.a); }
	void set(void* addr) const { *((uint32_t*)(addr))=bgra; }
} __attribute__((packed));
#undef CHANNEL_SCALE

static void* get_address(VESA::FrameBuffer* framebuffer, int x,int y) {
	#ifdef MOSS_DEBUG
	//Trying to access out of bounds causes the screen to be filled with magenta
	if (x<0 || x>=framebuffer->mode->info.XResolution) { framebuffer->draw_fill(Color(255u,0u,255u,255u)); return NULL; }
	if (y<0 || y>=framebuffer->mode->info.YResolution) { framebuffer->draw_fill(Color(255u,0u,255u,255u)); return NULL; }
	#endif

	y = framebuffer->mode->info.YResolution - y - 1;

	int bytes_per_pixel;
	switch (framebuffer->mode->info.BitsPerPixel) {
		default:
		case 4:
		case 8: //Unsupported
			bytes_per_pixel = 0;
			break;
		case 15: //Fallthrough, since one of the bits is actually unused.
		case 16:
			bytes_per_pixel = 2;
			break;
		case 24:
			bytes_per_pixel = 3;
			break;
		case 32:
			bytes_per_pixel = 4;
			break;
	}
	return (void*)( (unsigned long)(framebuffer->buffer) + y*framebuffer->mode->info.BytesPerScanLine + x*bytes_per_pixel );
}
static Color get_at(VESA::FrameBuffer* framebuffer, int x,int y) {
	const void* addr = get_address(framebuffer, x,y);
	switch (framebuffer->mode->info.BitsPerPixel) {
		case 4:
		case 8:
			break; //Not supported
		case 15: return Pixel15(addr).get();
		case 16: return Pixel16(addr).get();
		case 24: return Pixel24(addr).get();
		case 32: return Pixel32(addr).get();
	}
	return Color(255u,0u,255u,255u); //Error color is magenta
}
static void set_at(VESA::FrameBuffer* framebuffer, int x,int y, const Color& color) {
	void* addr = get_address(framebuffer, x,y);
	switch (framebuffer->mode->info.BitsPerPixel) {
		case 4:
		case 8:
			break; //Not supported
		case 15: Pixel15(color).set(addr); break;
		case 16: Pixel16(color).set(addr); break;
		case 24: Pixel24(color).set(addr); break;
		case 32: Pixel32(color).set(addr); break;
	}
}


} namespace VESA {


FrameBuffer::FrameBuffer(Mode* mode) : mode(mode),size(mode->info.BytesPerScanLine*mode->info.YResolution) {
	#ifdef MOSS_DEBUG
	#define MUST_SIZE(TYPE,SIZE)\
		if (sizeof(PixelUtil::TYPE)!=SIZE) {\
			char buffer[256];\
			MOSSC::sprintf(buffer,#TYPE" was the wrong size (expected "#SIZE", but is %u!)\n",sizeof(PixelUtil::TYPE));\
			/*Kernel::graphics->fill(Color(255u,0u,255u,255u));*/\
			ASSERT(false,buffer);\
		}
	MUST_SIZE(Pixel15,2)
	MUST_SIZE(Pixel16,2)
	MUST_SIZE(Pixel24,3)
	MUST_SIZE(Pixel32,4)
	#undef MUST_SIZE
	#endif

	buffer = MOSSC::malloc(size);

	complete = false;
}
FrameBuffer::~FrameBuffer(void) {
	MOSSC::free(buffer);
}

/*void set_vesa_bank(int bank_number) {
	Interrupts::regs16_t regs;
	regs.ax = 0x4F05;
	regs.bx = 0;
	regs.dx = bank_number;
	int32(0x10,&regs);
}*/
void FrameBuffer::draw_fill(const Color& color) {
	draw_rect(0,0,mode->info.XResolution,mode->info.YResolution, color);
}
void FrameBuffer::draw_rect(int x,int y,int w,int h, const Color& color) {
	for (int y2=0;y2<h;++y2) {
		for (int x2=0;x2<w;++x2) {
			set_pixel(x+x2,y+y2, color);
		}
	}
}

void FrameBuffer::draw_text(int x,int y, char text, const Color& color) {
	draw_text(x,y, text, color,Color(255,0,255,0));
}
void FrameBuffer::draw_text(int x,int y, char text, const Color& color,const Color& background) {
	uint64_t chr = Font::font[(unsigned int)(text)];
	uint64_t i = 0;
	for (int y2=y;y2<y+8;++y2) {
		for (int x2=x+8-1;x2>=x;--x2) {
			if (chr&(1ull<<i)) set_pixel(x2,y2,      color);
			else               set_pixel(x2,y2, background);
			++i;
		}
	}
}
void FrameBuffer::draw_text(int x,int y, const char* text, const Color& color) {
	draw_text(x,y, text, color,Color(255,0,255,0));
}
void FrameBuffer::draw_text(int x,int y, const char* text, const Color& color,const Color& background) {
	int i = 0;
	LOOP:
		char c = text[i];
		if (c=='\0') return;
		draw_text(x+8*i,y, c, color,background);
		++i;
		goto LOOP;
}

Color FrameBuffer::get_pixel(int x,int y) {
	//TODO: technically I think sometimes you aren't allowed to read from this memory?
	return PixelUtil::get_at(this, x,y);
}
void FrameBuffer::set_pixel(int x,int y, const Color& color) {
	/*int address = y*vesa_width + x;
	int bank_size = vesa_granularity*1024; //SVGA memory banks are not necessarily 64KiB, so be sure to use .WinGranularity.
	int bank_number = address/bank_size;
	int bank_offset = address%bank_size;

	set_vesa_bank(bank_number);

	*((int*)(0xA0000+bank_offset)) = color;*/

	return PixelUtil::set_at(this, x,y, color);
}

void FrameBuffer::blend_pixel(int x,int y, const Color& color) {
	Color original = get_pixel(x,y);

	Color new_color = Color::blend(color,original);

	set_pixel(x,y, new_color);
}

void FrameBuffer::copy_to_screen(const Mode* mode) const {
	MOSSC::memcpy((void*)(mode->info.PhysBasePtr),buffer,size);
}


}}}