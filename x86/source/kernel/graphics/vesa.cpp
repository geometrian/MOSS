#include "vesa.h"

#include "../../stdlib/stdlib.h"
#include "../interrupt/int32.h"
#include "../kernel.h"
#include "../text_mode_terminal.h"

#include "../../stdlib/math.h"

#include "font.h"


//See also http://www.brokenthorn.com/Resources/OSDevVid2.html


namespace MOSS { namespace Graphics { namespace VESA {


Mode::Mode(uint16_t index) : index(index) {
	MODE_INFO* info2 = (MODE_INFO*)(0x500);

	memset(info2,0,sizeof(MODE_INFO));
	int ptr = (int)(info2);

	Interrupts::regs16_t regs;
	regs.ax = 0x4F01;
	regs.di = ptr & 0xF;
	regs.es = (ptr>>4) & 0xFFFF;
	regs.cx = index;
	Interrupts::int32(0x10,&regs);

	valid = regs.ax==0x004F;

	memcpy(&info,info2,sizeof(MODE_INFO));
}
Mode::~Mode(void) {}

void Mode::print(Terminal::TextModeTerminal* terminal) const {
	terminal->write("Mode ");
	terminal->write(index);
	terminal->write(": ");
	terminal->write((int)(info.XResolution));
	terminal->write("x");
	terminal->write((int)(info.YResolution));
	terminal->write("@");
	terminal->write((int)(info.BitsPerPixel));
	terminal->write("bpp\n");
}


Controller::Controller(void) {
	width = -1;
	height = -1;
	current_mode = NULL;

	//Check that the VESA driver exists, and get information about it,
	//including how many modes there are.
	{
		VESA_INFO* info2 = (VESA_INFO*)(0x500);

		memset(info2,0,sizeof(VESA_INFO));
		strncpy((char*)(info2->VESASignature),"VBE2",4);
		int ptr = (int)(info2);

		//Kernel::terminal->write("Getting graphics modes . . .\n");
		Interrupts::regs16_t regs;
		regs.ax = 0x4F00;
		regs.di = ptr & 0xF;
		regs.es = (ptr>>4) & 0xFFFF;
		Interrupts::int32(0x10,&regs);

		//Kernel::terminal->write("Virtual pointer:   "); Kernel::terminal->write((void*)(vesa_info)); Kernel::terminal->write("\n");
		//Kernel::terminal->write("Segmented pointer: "); Kernel::terminal->write((void*)((ptr>>4) & 0xFFFF)); Kernel::terminal->write(":"); Kernel::terminal->write((void*)(ptr & 0xF)); Kernel::terminal->write("\n");

		ASSERT(regs.ax==0x004F,"Getting VESA graphics modes failed!");

		//Check that we got the right magic marker value
		ASSERT(strncmp((char*)(info2->VESASignature),"VESA",4)==0,"Got wrong VESA magic value!");

		//Copy the temporary information into this mode's record.
		memcpy(&info,info2,sizeof(VESA_INFO));

		//Kernel::terminal->write("Graphics modes retrieved!\n");
	}

	//Setup our list of modes
	{
		//Kernel::terminal->write("Listing graphics modes!\n");

		//Convert the mode list pointer from seg:offset to a linear address
		uint16_t* mode_ptr = (uint16_t*)( ((info.VideoModePtr&0xFFFF0000)>>12) + (info.VideoModePtr&0xFFFF) );
		uint16_t* mode_ptr2 = mode_ptr;

		//Kernel::terminal->write("Video pointer: ");
		//Kernel::terminal->write((void*)(mode_ptr));
		//Kernel::terminal->write("\n");

		//Kernel::terminal->write((int)(mode_ptr)); Kernel::terminal->write(" "); Kernel::terminal->write((int)(mode_ptr2)); Kernel::terminal->write("\n");

		numof_modes = 0;
		//Read the list of available modes
		LOOP: {
			uint16_t mode = mode_ptr[numof_modes];
			//Kernel::terminal->write((int)(mode)); Kernel::terminal->write(" ");
			if (mode!=0xFFFF) {
				++numof_modes;
				goto LOOP;
			}
		}
		//Kernel::terminal->write("\n");

		//Kernel::terminal->write((int)(mode_ptr)); Kernel::terminal->write(" "); Kernel::terminal->write((int)(mode_ptr2)); Kernel::terminal->write("\n");

		//Kernel::terminal->write("There are ");
		//Kernel::terminal->write(numof_modes);
		//Kernel::terminal->write(" modes!\n");
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
			//Kernel::terminal->write((int)(mode)); Kernel::terminal->write(" ");
			//Kernel::terminal->write("Making new mode "); Kernel::terminal->write((int)(mode)); Kernel::terminal->write("!\n");
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

		uint32_t score = abs(mode->info.XResolution*mode->info.YResolution-w*h) + 1000*abs(mode->info.BitsPerPixel-bpp);

		//Kernel::terminal->write((int)(score)); Kernel::terminal->write(": "); mode->print(Kernel::terminal); delay(1000);

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
void Controller::fill(unsigned char r,unsigned char g,unsigned char b,unsigned char a/*=255*/) {
	for (int y=0;y<height;++y) {
		for (int x=0;x<width;++x) {
			set_pixel(x,y, r,g,b,a);
		}
	}
}
void Controller::draw_text(int x,int y, char text, unsigned char r,unsigned char g,unsigned char b,unsigned char a/*=255*/) {
	uint64_t chr = Font::font[(unsigned int)(text)];
	int i = 0;
	for (int y2=y;y2<y+8;++y2) {
		for (int x2=x;x2<x+8;++x2) {
			if (chr&(1<<i)) draw_pixel(x2,y2, r,g,b,a);
		}
	}
}
void Controller::draw_text(int x,int y, const char* text, unsigned char r,unsigned char g,unsigned char b,unsigned char a/*=255*/) {
	int i = 0;
	LOOP:
		char c = text[i];
		if (c=='\0') return;
		Controller::draw_text(x+8*i,y, c, r,g,b,a);
		++i;
		goto LOOP;
}
void Controller::set_pixel(int x,int y, unsigned int rgba) {
	unsigned char r = (rgba&0xFF000000)>>24; //r
	unsigned char g = (rgba&0x00FF0000)>>16; //g
	unsigned char b = (rgba&0x0000FF00)>> 8; //b
	unsigned char a = (rgba&0x000000FF)    ; //a
	set_pixel(x,y, r,g,b,a);
}
void Controller::set_pixel(int x,int y, unsigned char r,unsigned char g,unsigned char b,unsigned char a/*=255*/) {
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

	addr[0] = b;
	addr[1] = g;
	addr[2] = r;
	addr[3] = a;
}


}}}