#pragma once

#include "../../../mosst/string.h"


namespace MOSS { namespace Graphics { namespace GUI {


class Frame;

class Manager {
	private:
		MOSST::Vector<Frame*> frames;

		int mouse_position[2];

	public:
		Manager(void);
		~Manager(void);

		void set_mouse_position(int x, int y);

		void add_frame(const MOSST::String& title, int x,int y,int w,int h);

		void draw(void) const;
};


}}}