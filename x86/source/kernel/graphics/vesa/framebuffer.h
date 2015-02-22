#pragma once


namespace MOSS { namespace Graphics {
	class Color;
namespace VESA {


class Mode;

class FrameBuffer {
	public:
		void* buffer;

		bool complete;

		Mode*const mode;
		const int size;

	public:
		FrameBuffer(Mode* mode);
		~FrameBuffer(void);

		void draw_fill(const Color& color);
		void draw_rect(int x,int y,int w,int h, const Color& color);

		void draw_text(int x,int y, char text, const Color& color);
		void draw_text(int x,int y, char text, const Color& color,const Color& background);
		void draw_text(int x,int y, const char* text, const Color& color);
		void draw_text(int x,int y, const char* text, const Color& color,const Color& background);

		Color get_pixel(int x,int y);
		void set_pixel(int x,int y, const Color& color);

		void blend_pixel(int x,int y, const Color& color);

		void copy_to_screen(const Mode* mode) const;
};


}}}