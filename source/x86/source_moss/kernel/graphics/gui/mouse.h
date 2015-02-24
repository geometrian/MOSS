#pragma once

#include "../../../includes.h"


namespace MOSS { namespace Graphics {
	namespace VESA {
		class Framebuffer;
	}
namespace GUI {


class Mouse final {
	public:
		int x,y;
		bool buttons[5];

	public:
		Mouse(void);
		~Mouse(void);

		void draw(VESA::Framebuffer* framebuffer) const;
};


}}}
