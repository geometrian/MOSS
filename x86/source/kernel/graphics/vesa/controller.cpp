#include "controller.h"

#include "../../../includes.h"

#include "../../../mossc/cstdlib"
#include "../../../mossc/cstring"

#include "../../interrupt/int32.h"
#include "../../io/io.h"

#include "framebuffer.h"
#include "mode.h"


//See also http://www.brokenthorn.com/Resources/OSDevVid2.html


namespace MOSS { namespace Graphics { namespace VESA {


Controller::Controller(void) {
	width = -1;
	height = -1;
	current_mode = NULL;

	frames[0] = NULL;
	frames[1] = NULL;
	latest_complete_frame = -1;
	frame_writing = 0;

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
	if (frames[1]!=NULL) delete frames[1];
	if (frames[0]!=NULL) delete frames[0];

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

	if (regs.ax!=0x004F) {
		ASSERT(false,"Could not set graphics mode!\n");
		return false;
	}

	width = mode->info.XResolution;
	height = mode->info.YResolution;
	current_mode = mode;

	if (frames[0]!=NULL) delete frames[0];
	if (frames[1]!=NULL) delete frames[1];
	frames[0] = new FrameBuffer(current_mode);
	frames[1] = new FrameBuffer(current_mode);

	return true;
}

void Controller::frame_start(void) {
	current_frame = frames[frame_writing];
	current_frame->complete = false;
}
void Controller::frame_end(void) {
	frames[frame_writing]->complete =  true;
	latest_complete_frame = frame_writing;
	frame_writing = 1 - frame_writing;
}
void Controller::frame_flip(void) {
	//See http://wiki.osdev.org/GUI
	//See http://www.delorie.com/djgpp/doc/ug/graphics/vga.html

	ASSERT(frames[0]!=NULL && frames[1]!=NULL, "At least one frame is NULL!  Must set a mode first!");
	if (latest_complete_frame==-1) return;

	//Wait for vertical retrace
	//	Wait for any previous retrace to end
	while (  IO::recv<uint8_t>(0x03DA)&0x08 );
	//Wait until a new retrace has just begun
	while (!(IO::recv<uint8_t>(0x03DA)&0x08));

	//Copy as fast as possible and hope we're done in time
	frames[latest_complete_frame]->copy_to_screen(current_mode);
}


}}}