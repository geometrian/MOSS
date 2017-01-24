#pragma once

#include "../../../includes.hpp"


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
