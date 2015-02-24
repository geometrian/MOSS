#pragma once

#include "../../../includes.h"


namespace MOSS { namespace Graphics {
	class Color;
namespace VESA {


class Mode;

class Framebuffer final {
	public:
		void* buffer;

		bool complete;

		Mode*const mode;
		const int size;

	public:
		Framebuffer(Mode* mode);
		~Framebuffer(void);

		void draw_fill(Color const& color);
		void draw_rect(int x,int y,int w,int h, Color const& color);

		void draw_text(int x,int y, char text, Color const& color);
		void draw_text(int x,int y, char text, Color const& color,Color const& background);
		void draw_text(int x,int y, char const* text, Color const& color);
		void draw_text(int x,int y, char const* text, Color const& color,Color const& background);

		void draw_line(int x0,int y0,int x1,int y1, Color const& color);

		Color get_pixel(int x,int y);
		void set_pixel(int x,int y, Color const& color);

		void blend_pixel(int x,int y, Color const& color);

		void copy_to_screen(const Mode* mode) const;
};


}}}
