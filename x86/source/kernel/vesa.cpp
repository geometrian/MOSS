#include "vesa.h"

#include "../stdlib/stdlib.h"
#include "interrupt/int32.h"
#include "kernel.h"
#include "text_mode_terminal.h"


//See also http://www.brokenthorn.com/Resources/OSDevVid2.html


namespace MOSS { namespace VESA {


bool get_vesa_info(VESA_INFO* vesa_info) {
	memset(vesa_info,0,sizeof(VESA_INFO));
	strncpy((char*)(vesa_info->VESASignature),"VBE2",4);
	int ptr = (int)(vesa_info);

	//terminal->write("Getting graphics modes . . .\n");
	Interrupts::regs16_t regs;
	regs.ax = 0x4F00;
	regs.di = ptr & 0xF;
	regs.es = (ptr>>4) & 0xFFFF;
	Interrupts::int32(0x10,&regs);

	//terminal->write("Virtual pointer:   "); terminal->write((void*)(vesa_info)); terminal->write("\n");
	//terminal->write("Segmented pointer: "); terminal->write((void*)((ptr>>4) & 0xFFFF)); terminal->write(":"); terminal->write((void*)(ptr & 0xF)); terminal->write("\n");

	if (regs.ax != 0x004F) {
		Kernel::terminal->write("Getting graphics modes failed!\n");
		return false;
	}

	//check that we got the right magic marker value
	if (strncmp((char*)(vesa_info->VESASignature),"VESA",4)!=0) {
		Kernel::terminal->write("Got wrong VESA magic value!\n");
		return false;
	}

	//terminal->write("Graphics modes retrieved!\n");

	return true;
}
int get_mode_info(MODE_INFO* mode_info, uint16_t mode) {
	memset(mode_info,0,sizeof(MODE_INFO));
	int ptr = (int)(mode_info);

	Interrupts::regs16_t regs;
	regs.ax = 0x4F01;
	regs.di = ptr & 0xF;
	regs.es = (ptr>>4) & 0xFFFF;
	regs.cx = mode;
	Interrupts::int32(0x10,&regs);

	if (regs.ax != 0x004F) {
		Kernel::terminal->write("Graphics mode info. failed for mode ");
		Kernel::terminal->write((int)(mode));
		Kernel::terminal->write("!\n");
		return -1;
	}

	return 0;
}

//These need to be below 1MiB or else they cannot be accessed through far pointers.
//We don't need them at the same time either, so point them to the same thing.  This
//address should be free for use after we get all our information from the bootloader.
//See http://wiki.osdev.org/Memory_Map_%28x86%29
VESA_INFO* vesa_info = (VESA_INFO*)(0x500);
MODE_INFO* mode_info = (MODE_INFO*)(0x500);

int find_vesa_mode(int w, int h, int depth) {
	//Check that the VESA driver exists, and get information about it
	if (!get_vesa_info(vesa_info)) return -1;

	/*terminal->write("Video pointer: ");
	terminal->write((void*)(vesa_info->VideoModePtr));
	terminal->write("\n");*/

	//Convert the mode list pointer from seg:offset to a linear address
	unsigned long mode_ptr = ((vesa_info->VideoModePtr&0xFFFF0000)>>12) + (vesa_info->VideoModePtr & 0xFFFF);

	//terminal->write("Listing graphics modes!\n");

	uint16_t mode_list[256];
	int number_of_modes = 0;
	//read the list of available modes
	LOOP: {
		if (number_of_modes==256) {
			Kernel::terminal->write("More than 256 modes!  Proceeding as if there were only 256!\n");
		} else {
			uint16_t mode = ((uint16_t*)(mode_ptr))[number_of_modes];
			if (mode!=0xFFFF) {
				mode_list[number_of_modes] = mode;
				++number_of_modes;
				mode_ptr += sizeof(uint16_t); //2
				goto LOOP;
			}
		}
	}

	/*terminal->write("There are ");
	terminal->write(number_of_modes);
	terminal->write(" modes!\n");*/

	//scan through the list of modes looking for the one that we want
	for (int c=0; c<number_of_modes; ++c) {
		//get information about this mode
		if (get_mode_info(mode_info,mode_list[c])!=0) continue;

		Kernel::terminal->write("Mode ");
		Kernel::terminal->write(c);
		Kernel::terminal->write(": ");
		Kernel::terminal->write((int)(mode_info->XResolution));
		Kernel::terminal->write("x");
		Kernel::terminal->write((int)(mode_info->YResolution));
		Kernel::terminal->write("@");
		Kernel::terminal->write((int)(mode_info->BitsPerPixel));
		Kernel::terminal->write("bpp\n");

		/*//check the flags field to make sure this is a color graphics mode, and that it is supported by the current hardware
		if ((mode_info.ModeAttributes & 0x19)!=0x19) {
			continue;
		}*/

		//check that this mode is the right size
		if ((mode_info->XResolution != w) || (mode_info->YResolution != h)) continue;

		////check that there is only one color plane
		//if (mode_info->NumberOfPlanes != 1) continue;

		////check that it is a packed-pixel mode (other values are used for different memory layouts, eg. 6 for a truecolor resolution)
		//if (mode_info->MemoryModel != 4) continue;

		//8-bit (256 color) mode, 24 and 32 are true color modes?
		if (mode_info->BitsPerPixel != depth) continue;

		//if it passed all those checks, this must be the mode we want!
		Kernel::terminal->write("Found satisfactory graphics mode!\n");
		return mode_list[c];
	}

	//oh dear, there was no mode matching the one we wanted!
	return -1;
}

int vesa_width;
int vesa_height;
MODE_INFO vesa_mode;

bool set_vesa_mode(int w, int h, int depth) {
	int mode_number = find_vesa_mode(w, h, depth);
	if (mode_number==-1) return false;

	Interrupts::regs16_t regs;
	regs.ax = 0x4F02;
	regs.bx = mode_number;
	Interrupts::int32(0x10,&regs);
	if (regs.ax != 0x004F) {
		Kernel::terminal->write("Could not set graphics mode!\n");
		return false;
	}

	vesa_width = w;
	vesa_height = h;
	vesa_mode = *mode_info;

	return true;
}

/*void set_vesa_bank(int bank_number) {
	Interrupts::regs16_t regs;
	regs.ax = 0x4F05;
	regs.bx = 0;
	regs.dx = bank_number;
	int32(0x10,&regs);
}*/
void set_vesa_pixel(int x,int y, unsigned int rgba) {
	unsigned char r = (rgba&0xFF000000)>>24; //r
	unsigned char g = (rgba&0x00FF0000)>>16; //g
	unsigned char b = (rgba&0x0000FF00)>> 8; //b
	unsigned char a = (rgba&0x000000FF)    ; //a
	set_vesa_pixel(x,y, r,g,b,a);
}
void set_vesa_pixel(int x,int y, unsigned char r,unsigned char g,unsigned char b,unsigned char a/*=255*/) {
	/*int address = y*vesa_width + x;
	int bank_size = vesa_granularity*1024; //SVGA memory banks are not necessarily 64KiB, so be sure to use .WinGranularity.
	int bank_number = address/bank_size;
	int bank_offset = address%bank_size;

	set_vesa_bank(bank_number);

	*((int*)(0xA0000+bank_offset)) = color;*/

	unsigned char* addr = (unsigned char*)( vesa_mode.PhysBasePtr + y*vesa_mode.BytesPerScanLine + x*vesa_mode.BitsPerPixel/8 );

	addr[0] = b;
	addr[1] = g;
	addr[2] = r;
	addr[3] = a;
}



#if 0
uint16_t find_mode(int x, int y, int d) {
	if (x==640 && y==480 && d==1) return 0x11;

	/*uint16_t best = 0x13;

	int pixdiff;
	int depthdiff;
	int bestpixdiff = DIFF(320*200, x*y);
	int bestdepthdiff = 8>=d ? 8-d : (d-8)*2;

	strncpy(ctrl->signature,"VBE2",4);*/

	unsigned short addr_ctrl = 0x2000;
	unsigned short addr_info = 0x3000;

	struct vbeControllerInfo* ctrl = (vbeControllerInfo*)((int)(addr_ctrl));
	struct       vbeModeInfo* info = (      vbeModeInfo*)((int)(addr_info));
	memset(ctrl, 0, sizeof(vbeControllerInfo));
	memset(info, 0, sizeof(      vbeModeInfo));
	strncpy(ctrl->signature, "VBE2", 4);

	//List all modes
	terminal->write("Listing graphics modes . . .\n");
	{
		regs16_t regs;
		regs.ax = 0x4F00;
		regs.es = 0;
		regs.di = addr_ctrl;
		int32(0x10,&regs);
		if (regs.ax!=0x004F) {
			terminal->write("Could not list graphics modes!\n");
		}
	}

	uint16_t* modes = (uint16_t*)(ctrl->videomodes);
	terminal->write("Examining graphics modes (from ");
	terminal->write((void*)(modes),'p');
	terminal->write(")\n");
	for (int i=0; modes[i]!=0xFFFF; ++i) {
		delay();

		terminal->write("Examining graphics mode ");
		terminal->write((void*)((int)(modes[i])),'p');
		terminal->write("\n");

		//Get Mode Info
		{
			regs16_t regs;
			regs.ax = 0x4F01;
			regs.cx = modes[i];
			regs.es = 0;
			regs.di = addr_info;
			int32(0x10,&regs);
			if (regs.ax!=0x004F) {
				//terminal->write("!");
				continue;
			}
		}

		terminal->write("Got valid mode!");

		//Check if this is a graphics mode with linear frame buffer support
		if ((info->attributes&0x90)!=0x90) continue;

		//Check if this is a packed pixel or direct color mode
		if (info->memory_model!=4 && info->memory_model!=6) continue;

		//Check if this is exactly the mode we're looking for
		if (x==info->Xres && y==info->Yres && d==info->bpp) return modes[i];

		//Otherwise, compare to the closest match so far, remember if best
		/*pixdiff = DIFF(inf->Xres*inf->Yres, x*y);
		depthdiff = (inf->bpp>=d)? inf->bpp-d : (d-inf->bpp)*2;
		if (bestpixdiff>pixdiff || (bestpixdiff==pixdiff&&bestdepthdiff>depthdiff)) {
			best = modes[i];
			bestpixdiff = pixdiff;
			bestdepthdiff = depthdiff;
		}*/
	}

	terminal->write("No exact modes found!\n");

	return 0x13;
}
#endif


}}