#pragma once

#include "../../../mosst/string.h"


namespace MOSS { namespace Graphics {
	namespace VESA {
		class FrameBuffer;
	}
namespace GUI {


class Frame;

class Mouse;

class Manager {
	private:
		MOSST::Vector<Frame*> frames;

		Mouse* mouse;

	public:
		Manager(void);
		~Manager(void);

		void set_mouse_position(int x, int y);

		void add_frame(const MOSST::String& title, int x,int y,int w,int h);

		void draw(VESA::FrameBuffer* framebuffer) const;
};


}}}