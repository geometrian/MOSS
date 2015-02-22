#include "vesa.h"

#include "../../mossc/cstdlib"
#include "../../mossc/cstring"
#include "../interrupt/int32.h"
#include "../kernel.h"
#include "../text_mode_terminal.h"

#include "color.h"
#include "font.h"


//See also http://www.brokenthorn.com/Resources/OSDevVid2.html


namespace MOSS { namespace Graphics { namespace VESA {


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

void Mode::print(Terminal::TextModeTerminal* terminal) const {
	terminal->write("Mode %u: %ux%u@%ubpp", index, info.XResolution,info.YResolution,info.BitsPerPixel);
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
	if (regs.ax != 0x004F) {
		//Kernel::terminal->write("Could not set graphics mode!\n");
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
	for (int y=0;y<height;++y) {
		for (int x=0;x<width;++x) {
			set_pixel(x,y, color);
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
void Controller::set_pixel(int x,int y, const Color& color) {
	if (x<0 || x>= width) return;
	if (y<0 || y>=height) return;

	y = height - y - 1;

	/*int address = y*vesa_width + x;
	int bank_size = vesa_granularity*1024; //SVGA memory banks are not necessarily 64KiB, so be sure to use .WinGranularity.
	int bank_number = address/bank_size;
	int bank_offset = address%bank_size;

	set_vesa_bank(bank_number);

	*((int*)(0xA0000+bank_offset)) = color;*/

	unsigned char* addr = (unsigned char*)( current_mode->info.PhysBasePtr + y*current_mode->info.BytesPerScanLine + x*current_mode->info.BitsPerPixel/8 );

	addr[0] = color.b;
	addr[1] = color.g;
	addr[2] = color.r;
	addr[3] = color.a;
}
void Controller::blend_pixel(int x,int y, const Color& color) {
	if (x<0 || x>= width) return;
	if (y<0 || y>=height) return;

	y = height - y - 1;

	unsigned char* addr = (unsigned char*)( current_mode->info.PhysBasePtr + y*current_mode->info.BytesPerScanLine + x*current_mode->info.BitsPerPixel/8 );

	Color original;
	original.b = addr[0];
	original.g = addr[1];
	original.r = addr[2];
	original.a = addr[3];

	//TODO: technically I think sometimes you aren't allowed to read from this memory?
	Color new_color = Color::blend(color,original);
	addr[0] = new_color.b;
	addr[1] = new_color.g;
	addr[2] = new_color.r;
	addr[3] = new_color.a;
}


}}}