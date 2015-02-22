#pragma once


namespace MOSS { namespace Graphics {
	namespace VESA {
		class Framebuffer;
	}
namespace GUI {


class Mouse {
	public:
		int x,y;
		bool buttons[5];

	public:
		Mouse(void);
		~Mouse(void);

		void draw(VESA::Framebuffer* framebuffer) const;
};


}}}