#pragma once

#include "../../../mosst/string.h"

#include "../../kernel.h"

#include "../color.h"

#include "../vesa/controller.h"
#include "../vesa/framebuffer.h"


namespace MOSS { namespace Graphics { namespace GUI {


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
		Frame(int x,int y, int w,int h) {
			set_position(x,y);
			set_size(w,h);
			visible = true; //TODO: false
		}
		~Frame(void) {
		}

		void set_position(int x, int y) {
			params_window.x = x;
			params_window.y = y;
		}
		void set_size(int width, int height) {
			params_window.w =  width;
			params_window.h = height;
		}

		inline void set_title(const MOSST::String& title) {
			this->title = title;
		}

		inline void set_visible(bool visible) {
			this->visible = visible;
		}

		void draw(void) const {
			Kernel::graphics->current_frame->draw_rect(params_window.x,params_window.y,params_window.w,params_window.h, Graphics::Color(0,64,64,255));
		}
};


}}}