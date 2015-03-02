#include "framebuffer.h"

#include "../../../mossc/cmath"
#include "../../../mossc/cstdio"
#include "../../../mossc/cstdlib"
#include "../../../mossc/cstring"

#include "../color.h"
#include "../font.h"

#include "mode.h"


namespace MOSS { namespace Graphics { namespace PixelUtil {


//See http://wiki.osdev.org/Bochs_Graphics_Adaptor#Memory_layout_of_video_modes
class _Pixel15Channels final { public:
	uint32_t      b : 5;
	uint32_t      g : 5;
	uint32_t      r : 5;
	uint32_t unused : 1;
} __attribute__((packed));
class _Pixel16Channels final { public:
	uint32_t b : 5;
	uint32_t g : 6;
	uint32_t r : 5;
} __attribute__((packed));
#define CHANNEL_SCALE(CHANNEL, SRC,DST, MAX_SRC_SIZE,MAX_DST_SIZE)\
	DST.CHANNEL = (uint32_t)( MAX_DST_SIZE##.0f*(float)(SRC.CHANNEL)/MAX_SRC_SIZE##.0f )
class Pixel15 final { public:
	union {
		_Pixel15Channels channels;
		uint16_t color;
	};
	inline Pixel15(Color const& color) {  CHANNEL_SCALE(r, color,channels, 255,31);  CHANNEL_SCALE(g, color,channels, 255,31);  CHANNEL_SCALE(b, color,channels, 255,31);  channels.unused=0u;  }
	inline Pixel15(void const* addr) : color(*( (uint16_t*)(addr) )) {}
	inline Color get(void) const {
		Color result;
		CHANNEL_SCALE(r, channels,result, 31,255);  CHANNEL_SCALE(g, channels,result, 31,255);  CHANNEL_SCALE(b, channels,result, 31,255);
		result.a = 255u;
		return result;
	}
	inline void set(void* addr) const { *((uint16_t*)(addr))=color; }
};
static_assert(sizeof(PixelUtil::Pixel15)==2,"Pixel15 is the wrong size!");
class Pixel16 final { public:
	union {
		_Pixel16Channels channels;
		uint16_t color;
	};
	inline Pixel16(Color const& color) {  CHANNEL_SCALE(r, color,channels, 255,31);  CHANNEL_SCALE(g, color,channels, 255,63);  CHANNEL_SCALE(b, color,channels, 255,31);  }
	inline Pixel16(void const* addr) : color(*( (uint16_t*)(addr) )) {}
	inline Color get(void) const {
		Color result;
		CHANNEL_SCALE(r, channels,result, 31,255);  CHANNEL_SCALE(g, channels,result, 63,255);  CHANNEL_SCALE(b, channels,result, 31,255);
		result.a = 255u;
		return result;
	}
	inline void set(void* addr) const { *((uint16_t*)(addr))=color; }
};
static_assert(sizeof(PixelUtil::Pixel16)==2,"Pixel16 is the wrong size!");
class Pixel24 final { public:
	struct {
		uint8_t b;
		uint8_t g;
		uint8_t r;
	} channels;
	inline Pixel24(Color const& color) { channels.r=color.r; channels.g=color.g; channels.b=color.b; }
	inline Pixel24(void const* addr) { unsigned char const* addr2=(unsigned char const*)(addr); channels.r=addr2[2]; channels.g=addr2[1]; channels.b=addr2[0]; }
	inline Color get(void) const { return Color(channels.r,channels.g,channels.b,255u); }
	inline void set(void* addr) const { unsigned char* addr2=(unsigned char*)(addr); addr2[2]=channels.r; addr2[1]=channels.g; addr2[0]=channels.b; }
} __attribute__((packed));
static_assert(sizeof(PixelUtil::Pixel24)==3,"Pixel24 is the wrong size!");
class Pixel32 final { public:
	union {
		struct {
			uint8_t b;
			uint8_t g;
			uint8_t r;
			uint8_t a;
		} channels;
		uint32_t bgra;
	};
	inline Pixel32(Color const& color) { channels.r=color.r; channels.g=color.g; channels.b=color.b; channels.a=color.a; }
	inline Pixel32(void const* addr) : bgra(*( (uint32_t*)(addr) )) {}
	inline Color get(void) const { return Color(channels.r,channels.g,channels.b,channels.a); }
	inline void set(void* addr) const { *((uint32_t*)(addr))=bgra; }
} __attribute__((packed));
static_assert(sizeof(PixelUtil::Pixel32)==4,"Pixel32 is the wrong size!");
#undef CHANNEL_SCALE

static void* get_address(VESA::Framebuffer* framebuffer, int x,int y) {
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
static Color get_at(VESA::Framebuffer* framebuffer, int x,int y) {
	void const* addr = get_address(framebuffer, x,y);
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
static void set_at(VESA::Framebuffer* framebuffer, int x,int y, Color const& color) {
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


Framebuffer::Framebuffer(Mode* mode) : mode(mode),size(mode->info.BytesPerScanLine*mode->info.YResolution) {
	buffer = MOSSC::malloc(size);

	complete = false;
}
Framebuffer::~Framebuffer(void) {
	MOSSC::free(buffer);
}

/*void set_vesa_bank(int bank_number) {
	Interrupts::regs16_t regs;
	regs.ax = 0x4F05;
	regs.bx = 0;
	regs.dx = bank_number;
	int32(0x10,&regs);
}*/
void Framebuffer::draw_fill(Color const& color) {
	draw_rect(0,0,mode->info.XResolution,mode->info.YResolution, color);
}
void Framebuffer::draw_rect(int x,int y,int w,int h, Color const& color) {
	int x_start = max(x,0);
	int y_start = max(y,0);
	int x_end = min(x+w,(int)(mode->info.XResolution));
	int y_end = min(y+h,(int)(mode->info.YResolution));
	for (int y2=y_start;y2<y_end;++y2) {
		for (int x2=x_start;x2<x_end;++x2) {
			set_pixel(x2,y2, color);
		}
	}
}

void Framebuffer::draw_text(int x,int y, char character, Color const& color) {
	draw_text(x,y, character, color,Color(255,0,255,0));
}
void Framebuffer::draw_text(int x,int y, char character, Color const& color,Color const& background) {
	Font::Character8x8 const& chr = Font::font8x8[static_cast<int>(character)];

	int x_start = max(x,0);
	int y_start = max(y,0);
	int x_end = min(x+8,mode->info.XResolution-1);
	int y_end = min(y+8,mode->info.YResolution-1);
	for (int y2=y_start;y2<y_end;++y2) {
		uint8_t const& row = chr.rows[y2-y_start];
		for (int x2=x_start;x2<x_end;++x2) {
			int x3 = x2 - x_start;
			if (row&(1<<(7-x3))) blend_pixel(x2,y2,      color);
			else                 blend_pixel(x2,y2, background);
		}
	}
}
void Framebuffer::draw_text(int x,int y, char const* text, Color const& color) {
	draw_text(x,y, text, color,Color(255,0,255,0));
}
void Framebuffer::draw_text(int x,int y, char const* text, Color const& color,Color const& background) {
	int i = 0;
	LOOP:
		char c = text[i];
		if (c=='\0') return;
		draw_text(x+8*i,y, c, color,background);
		++i;
		goto LOOP;
}

void Framebuffer::draw_line(int x0,int y0,int x1,int y1, Color const& color) {
	//http://tech-algorithm.com/articles/drawing-line-using-bresenham-algorithm/
	int x = x0;
	int y = y0;

	int w = x1 - x;
	int h = y1 - y;

	int dx0=0, dy0=0, dx1=0, dy1=0;
	if (w<0) dx0=-1; else if (w>0) dx0=1;
	if (h<0) dy0=-1; else if (h>0) dy0=1;
	if (w<0) dx1=-1; else if (w>0) dx1=1;

	int longest = abs(w);
	int shortest = abs(h);
	if (!(longest>shortest)) {
		longest = abs(h);
		shortest = abs(w);
		if (h<0) dy1=-1; else if (h>0) dy1=1;
		dx1 = 0;
	}
	int numerator = longest >> 1;
	for (int i=longest;i>=0;--i) {
		if (x<0||x>=mode->info.XResolution) continue;
		if (y<0||y>=mode->info.YResolution) continue;
		blend_pixel(x,y, color);

		numerator += shortest;
		if (!(numerator<longest)) {
			numerator -= longest;
			x += dx0;
			y += dy0;
		} else {
			x += dx1;
			y += dy1;
		}
	}
}

Color Framebuffer::get_pixel(int x,int y) {
	#ifdef MOSS_DEBUG
	//Trying to access out of bounds causes the screen to be filled with magenta
	if (x<0 || x>=mode->info.XResolution) { draw_fill(Color(255u,0u,255u,255u)); return Color(); }
	if (y<0 || y>=mode->info.YResolution) { draw_fill(Color(255u,0u,255u,255u)); return Color(); }
	#endif

	//TODO: technically I think sometimes you aren't allowed to read from this memory?
	return PixelUtil::get_at(this, x,y);
}
void Framebuffer::set_pixel(int x,int y, Color const& color) {
	//Some drawing operations do this, and it's much more convenient to just discard
	//the pixels here than to check them in each higher level call.
	if (x<0 || x>=mode->info.XResolution) return;
	if (y<0 || y>=mode->info.YResolution) return;

	return PixelUtil::set_at(this, x,y, color);
}

void Framebuffer::blend_pixel(int x,int y, Color const& color) {
	//Some drawing operations do this, and it's much more convenient to just discard
	//the pixels here than to check them in each higher level call.
	if (x<0 || x>=mode->info.XResolution) return;
	if (y<0 || y>=mode->info.YResolution) return;

	Color original = get_pixel(x,y);

	Color new_color = Color::blend(color,original);

	set_pixel(x,y, new_color);
}

void Framebuffer::copy_to_screen(const Mode* mode) const {
	MOSSC::memcpy((void*)(mode->info.PhysBasePtr),buffer,size);
}


}}}
