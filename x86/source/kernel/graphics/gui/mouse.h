#pragma once


namespace MOSS { namespace Graphics {
	namespace VESA {
		class FrameBuffer;
	}
namespace GUI {


class FrameBuffer;

class Mouse {
	public:
		int x,y;

	public:
		Mouse(void);
		~Mouse(void);

		void draw(VESA::FrameBuffer* framebuffer) const;
};


}}}