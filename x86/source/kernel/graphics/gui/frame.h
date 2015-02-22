#pragma once

#include "../../../mosst/string.h"


namespace MOSS { namespace Graphics {
	namespace VESA {
		class FrameBuffer;
	}
namespace GUI {


class Rect { public:
	int x,y, w,h;
};

class Frame {
	private:
		Rect params_window;
		Rect params_client;
		MOSST::String title;
		bool visible;

	public:
		Frame(int x,int y, int w,int h);
		~Frame(void);

		void set_position(int x, int y);
		void set_size(int width, int height);

		void set_title(const MOSST::String& title);

		void set_visible(bool visible);

		void draw(VESA::FrameBuffer* framebuffer) const;
};


}}}