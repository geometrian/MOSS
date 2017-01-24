#include "controller.hpp"

#include "../../../mossc/cstdlib"
#include "../../../mossc/cstring"

#include "../../interrupt/int32.hpp"
#include "../../io/io.hpp"

#include "framebuffer.hpp"
#include "mode.hpp"


//See also http://www.brokenthorn.com/Resources/OSDevVid2.html


namespace MOSS { namespace Graphics { namespace VESA {


Controller::Controller(void) {
	width = -1;
	height = -1;
	current_mode = nullptr;

	framebuffers[0] = nullptr;
	framebuffers[1] = nullptr;
	latest_complete_framebuffer = -1;
	framebuffer_writing = 0;

	//Check that the VESA driver exists, and get information about it,
	//including how many modes there are.
	{
		VESA_INFO* info2 = (VESA_INFO*)(0x500);

		MOSSC::memset(info2,0,sizeof(VESA_INFO));
		MOSSC::strncpy((char*)(info2->VESASignature),"VBE2",4);
		int ptr = (int)(info2);

		//kernel->write("Getting graphics modes . . .\n");
		Interrupts::regs16_t regs;
		regs.ax = 0x4F00;
		regs.di = ptr & 0xF;
		regs.es = (ptr>>4) & 0xFFFF;
		Interrupts::int32(0x10,&regs);

		//kernel->write("Virtual pointer:   %p\n",vesa_info);
		//kernel->write("Segmented pointer: %p:%p\n",(ptr>>4)&0xFFFF,ptr&0xF);

		assert_term(regs.ax==0x004F,"Getting VESA graphics modes failed!");

		//Check that we got the right magic marker value
		assert_term(MOSSC::strncmp(reinterpret_cast<char*>(info2->VESASignature),"VESA",4)==0,"Got wrong VESA magic value!");

		//Copy the temporary information into this mode's record.
		MOSSC::memcpy(&info,info2,sizeof(VESA_INFO));

		//kernel->write("Graphics modes retrieved!\n");
	}

	//Setup our list of modes
	{
		//kernel->write("Listing graphics modes!\n");

		//Convert the mode list pointer from seg:offset to a linear address
		uint16_t* mode_ptr = (uint16_t*)( ((info.VideoModePtr&0xFFFF0000)>>12) + (info.VideoModePtr&0xFFFF) );
		uint16_t* mode_ptr2 = mode_ptr;

		//kernel->write("Video pointer: %p\n",mode_ptr);

		//kernel->write("%d %d\n",mode_ptr,mode_ptr2);

		numof_modes = 0;
		//Read the list of available modes
		LOOP: {
			uint16_t mode = mode_ptr[numof_modes];
			//kernel->write("%d ",mode);
			if (mode!=0xFFFF) {
				++numof_modes;
				goto LOOP;
			}
		}
		//kernel->write("\n");

		//kernel->write("%d %d\n",mode_ptr,mode_ptr2);

		//kernel->write("There are %d modes!\n",numof_modes);
		assert_term(numof_modes>=1,"No VESA modes available!"); //A problem since don't want to try to allocate nothing.

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
			//kernel->write("Making new mode %d!\n",mode);
			modes[i] = new Mode(mode);
		}

		delete [] mode_indices;
	}
}
Controller::~Controller(void) {
	if (framebuffers[1]!=nullptr) delete framebuffers[1];
	if (framebuffers[0]!=nullptr) delete framebuffers[0];

	delete [] modes;
}

Mode* Controller::get_mode_closest(int w,int h, int bpp) {
	Mode* best = nullptr;
	uint32_t best_score = -1;

	for (int i=0;i<numof_modes;++i) {
		Mode* mode = modes[i];

		//Only linear frame buffer support is supported by MOSS.
		if (!(mode->info.ModeAttributes&0x90)) continue;

		uint32_t score = MOSSC::abs(mode->info.XResolution*mode->info.YResolution-w*h) + 1000*MOSSC::abs(mode->info.BitsPerPixel-bpp);

		//kernel->write("%d: ",score); mode->print(); delay(1000);

		if (score<best_score) {
			//kernel->write("  BEST SO FAR!\n");
			best = mode;
			best_score = score;
		}
	}

	assert_term(best!=nullptr,"No satisfactory VESA mode!");
	return best;
}

bool Controller::set_mode(Mode* mode) {
	Interrupts::regs16_t regs;
	regs.ax = 0x4F02;
	regs.bx = mode->index | 0x4000; //http://forum.osdev.org/viewtopic.php?f=1&t=26929
	Interrupts::int32(0x10,&regs);

	if (regs.ax!=0x004F) {
		assert_term(false,"Could not set graphics mode!\n");
		return false;
	}

	width = mode->info.XResolution;
	height = mode->info.YResolution;
	current_mode = mode;

	if (framebuffers[0]!=nullptr) delete framebuffers[0];
	if (framebuffers[1]!=nullptr) delete framebuffers[1];
	framebuffers[0] = new Framebuffer(current_mode);
	framebuffers[1] = new Framebuffer(current_mode);

	return true;
}

void Controller::frame_start(void) {
	current_framebuffer = framebuffers[framebuffer_writing];
	current_framebuffer->complete = false;
}
void Controller::frame_end(void) {
	framebuffers[framebuffer_writing]->complete =  true;
	latest_complete_framebuffer = framebuffer_writing;
	framebuffer_writing = 1 - framebuffer_writing;
}
void Controller::frame_flip(void) {
	//See http://wiki.osdev.org/GUI
	//See http://www.delorie.com/djgpp/doc/ug/graphics/vga.html

	assert_term(framebuffers[0]!=nullptr && framebuffers[1]!=nullptr, "At least one frame is null!  Must set a mode first!");
	if (latest_complete_framebuffer==-1) return;

	//Wait for vertical retrace
	//	Wait for any previous retrace to end
	while (  IO::recv<uint8_t>(0x03DA)&0x08 );
	//Wait until a new retrace has just begun
	while (!(IO::recv<uint8_t>(0x03DA)&0x08));

	//Copy as fast as possible and hope we're done in time
	framebuffers[latest_complete_framebuffer]->copy_to_screen(current_mode);
}


}}}
