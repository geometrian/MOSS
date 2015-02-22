#include "vesa.h"

#include "../../mossc/cstring"
#include "../../mossc/cstdio"
#include "../../mossc/cstdlib"
#include "../interrupt/int32.h"
#include "../kernel.h"
#include "../text_mode_terminal.h"

#include "color.h"
#include "font.h"


//See also http://www.brokenthorn.com/Resources/OSDevVid2.html


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

static void* get_address(VESA::Mode* mode, int x,int y) {
	#ifdef MOSS_DEBUG
	//Trying to access out of bounds causes the screen to be filled with magenta
	if (x<0 || x>=mode->info.XResolution) { Kernel::graphics->fill(Color(255u,0u,255u,255u)); return NULL; }
	if (y<0 || y>=mode->info.YResolution) { Kernel::graphics->fill(Color(255u,0u,255u,255u)); return NULL; }
	#endif

	y = mode->info.YResolution - y - 1;

	int bytes_per_pixel;
	switch (mode->info.BitsPerPixel) {
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
	return (void*)( mode->info.PhysBasePtr + y*mode->info.BytesPerScanLine + x*bytes_per_pixel );
}
static Color get_at(VESA::Mode* mode, int x,int y) {
	const void* addr = get_address(mode, x,y);
	switch (mode->info.BitsPerPixel) {
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
static void set_at(VESA::Mode* mode, int x,int y, const Color& color) {
	void* addr = get_address(mode, x,y);
	switch (mode->info.BitsPerPixel) {
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


Mode::Mode(uint16_t index) : index(index) {
	MODE_INFO* info2 = (MODE_INFO*)(0x500);

	MOSSC::memset(info2,0,sizeof(MODE_INFO));
	int ptr = (int)(info2);

	Interrupts::regs16_t regs;
	regs.ax = 0x4F01;
	regs.di = ptr & 0xF;
	regs.es = (ptr>>4) & 0xFFFF;
	regs.cx = index;
	Interrupts::int32(0x10,&regs);

	valid = regs.ax==0x004F;

	MOSSC::memcpy(&info,info2,sizeof(MODE_INFO));
}
Mode::~Mode(void) {}

void Mode::get_printable(char* buffer) const {
	MOSSC::sprintf(buffer,"Mode %u: %ux%u@%ubpp", index, info.XResolution,info.YResolution,info.BitsPerPixel);
}
void Mode::print(Terminal::TextModeTerminal* terminal) const {
	char buffer[256];
	get_printable(buffer);
	terminal->write(buffer);
}


Controller::Controller(void) {
	width = -1;
	height = -1;
	current_mode = NULL;

	//Check that the VESA driver exists, and get information about it,
	//including how many modes there are.
	{
		VESA_INFO* info2 = (VESA_INFO*)(0x500);

		MOSSC::memset(info2,0,sizeof(VESA_INFO));
		MOSSC::strncpy((char*)(info2->VESASignature),"VBE2",4);
		int ptr = (int)(info2);

		//Kernel::terminal->write("Getting graphics modes . . .\n");
		Interrupts::regs16_t regs;
		regs.ax = 0x4F00;
		regs.di = ptr & 0xF;
		regs.es = (ptr>>4) & 0xFFFF;
		Interrupts::int32(0x10,&regs);

		//Kernel::terminal->write("Virtual pointer:   %p\n",vesa_info);
		//Kernel::terminal->write("Segmented pointer: %p:%p\n",(ptr>>4)&0xFFFF,ptr&0xF);

		ASSERT(regs.ax==0x004F,"Getting VESA graphics modes failed!");

		//Check that we got the right magic marker value
		ASSERT(MOSSC::strncmp((char*)(info2->VESASignature),"VESA",4)==0,"Got wrong VESA magic value!");

		//Copy the temporary information into this mode's record.
		MOSSC::memcpy(&info,info2,sizeof(VESA_INFO));

		//Kernel::terminal->write("Graphics modes retrieved!\n");
	}

	//Setup our list of modes
	{
		//Kernel::terminal->write("Listing graphics modes!\n");

		//Convert the mode list pointer from seg:offset to a linear address
		uint16_t* mode_ptr = (uint16_t*)( ((info.VideoModePtr&0xFFFF0000)>>12) + (info.VideoModePtr&0xFFFF) );
		uint16_t* mode_ptr2 = mode_ptr;

		//Kernel::terminal->write("Video pointer: %p\n",mode_ptr);

		//Kernel::terminal->write("%d %d\n",mode_ptr,mode_ptr2);

		numof_modes = 0;
		//Read the list of available modes
		LOOP: {
			uint16_t mode = mode_ptr[numof_modes];
			//Kernel::terminal->write("%d ",mode);
			if (mode!=0xFFFF) {
				++numof_modes;
				goto LOOP;
			}
		}
		//Kernel::terminal->write("\n");

		//Kernel::terminal->write("%d %d\n",mode_ptr,mode_ptr2);

		//Kernel::terminal->write("There are %d modes!\n",numof_modes);
		ASSERT(numof_modes>=1,"No VESA modes available!"); //A problem since .get_mode_closest(...) cannot return NULL.

		//We have to go through an intermediary step of caching the mode numbers here since they are originally
		//stored low in memory, and creating a new mode immediately (and thereby retrieving information about it)
		//overwrites this lower memory.
		uint16_t* mode_indices = new uint16_t[numof_modes];
		for (int i=0;i<numof_modes;++i) {
			mode_indices[i] = mode_ptr2[i];
		}

		modes = new Mode*[numof_modes];
		for (int i=0;i<numof_modes;++i) {
			uint16_t mode = mode_indices[i];
			//Kernel::terminal->write("Making new mode %d!\n",mode);
			modes[i] = new Mode(mode);
		}

		delete [] mode_indices;
	}
}
Controller::~Controller(void) {
	delete [] modes;
}

Mode* Controller::get_mode_closest(int w,int h, int bpp) {
	#ifdef MOSS_DEBUG
	#define MUST_SIZE(TYPE,SIZE)\
		if (sizeof(PixelUtil::TYPE)!=SIZE) {\
			Kernel::terminal->write(#TYPE" was the wrong size (expected "#SIZE", but is %u!)\n",sizeof(PixelUtil::TYPE));\
			/*Kernel::graphics->fill(Color(255u,0u,255u,255u));*/\
			ASSERT(false,"");\
		}
	MUST_SIZE(Pixel15,2)
	MUST_SIZE(Pixel16,2)
	MUST_SIZE(Pixel24,3)
	MUST_SIZE(Pixel32,4)
	#undef MUST_SIZE
	#endif

	Mode* best = NULL;
	uint32_t best_score = -1;

	for (int i=0;i<numof_modes;++i) {
		Mode* mode = modes[i];

		uint32_t score = MOSSC::abs(mode->info.XResolution*mode->info.YResolution-w*h) + 1000*MOSSC::abs(mode->info.BitsPerPixel-bpp);

		//Kernel::terminal->write("%d: ",score); mode->print(Kernel::terminal); delay(1000);

		if (score<best_score) {
			//Kernel::terminal->write("  BEST SO FAR!\n");
			best = mode;
			best_score = score;
		}
	}

	return best;
}

bool Controller::set_mode(Mode* mode) {
	Interrupts::regs16_t regs;
	regs.ax = 0x4F02;
	regs.bx = mode->index;
	Interrupts::int32(0x10,&regs);

	if (regs.ax!=0x004F) {
		ASSERT(false,"Could not set graphics mode!\n");
		return false;
	}

	width = mode->info.XResolution;
	height = mode->info.YResolution;
	current_mode = mode;

	return true;
}


/*void set_vesa_bank(int bank_number) {
	Interrupts::regs16_t regs;
	regs.ax = 0x4F05;
	regs.bx = 0;
	regs.dx = bank_number;
	int32(0x10,&regs);
}*/
void Controller::fill(const Color& color) {
	draw_rect(0,0,width,height, color);
}
void Controller::draw_rect(int x,int y,int w,int h, const Color& color) {
	for (int y2=0;y2<h;++y2) {
		for (int x2=0;x2<w;++x2) {
			set_pixel(x+x2,y+y2, color);
		}
	}
}
void Controller::draw_text(int x,int y, char text, const Color& color) {
	draw_text(x,y, text, color,Color(255,0,255,0));
}
void Controller::draw_text(int x,int y, char text, const Color& color,const Color& background) {
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
void Controller::draw_text(int x,int y, const char* text, const Color& color) {
	draw_text(x,y, text, color,Color(255,0,255,0));
}
void Controller::draw_text(int x,int y, const char* text, const Color& color,const Color& background) {
	int i = 0;
	LOOP:
		char c = text[i];
		if (c=='\0') return;
		Controller::draw_text(x+8*i,y, c, color,background);
		++i;
		goto LOOP;
}
Color Controller::get_pixel(int x,int y) {
	//TODO: technically I think sometimes you aren't allowed to read from this memory?
	return PixelUtil::get_at(current_mode, x,y);
}
void Controller::set_pixel(int x,int y, const Color& color) {
	/*int address = y*vesa_width + x;
	int bank_size = vesa_granularity*1024; //SVGA memory banks are not necessarily 64KiB, so be sure to use .WinGranularity.
	int bank_number = address/bank_size;
	int bank_offset = address%bank_size;

	set_vesa_bank(bank_number);

	*((int*)(0xA0000+bank_offset)) = color;*/

	return PixelUtil::set_at(current_mode, x,y, color);
}
void Controller::blend_pixel(int x,int y, const Color& color) {
	Color original = get_pixel(x,y);

	Color new_color = Color::blend(color,original);

	set_pixel(x,y, new_color);
}


}}}