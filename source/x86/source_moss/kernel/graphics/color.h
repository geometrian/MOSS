#pragma once

#include "../../includes.h"


namespace MOSS { namespace Graphics {


class Color { public:
	union {
		struct {
			unsigned char r,g,b,a;
		};
		unsigned int rgba;
	};

	inline Color(void) {}
	inline Color(unsigned char r,unsigned char g,unsigned char b,unsigned char a=255) : r(r),g(g),b(b),a(a) {}

	static Color blend(Color const& foreground, Color const& background) {
		//#define ROUND(X) ((unsigned char)(X+0.5f))

		Color result;

		#if 0
			float alpha = (float)(foreground.a) / 255.0f;
			float alpha1 = 1.0f - alpha;
			result.r = (unsigned char)(foreground.r*alpha + background.r*alpha1);
			result.g = (unsigned char)(foreground.g*alpha + background.g*alpha1);
			result.b = (unsigned char)(foreground.b*alpha + background.b*alpha1);
			result.a = 255u;
		#else
			//http://stackoverflow.com/questions/12011081/alpha-blending-2-rgba-colors-in-c
			//TODO: http://stackoverflow.com/questions/5438313/faster-alpha-blending-using-a-lookup-table?
			unsigned int alpha = foreground.a + 1;
			unsigned int inv_alpha = 256 - foreground.a; //yes, 256
			result.r = (unsigned char)( (alpha*foreground.r + inv_alpha*background.r) >> 8);
			result.g = (unsigned char)( (alpha*foreground.g + inv_alpha*background.g) >> 8);
			result.b = (unsigned char)( (alpha*foreground.b + inv_alpha*background.b) >> 8);
			result.a = 0xFF;
		#endif

		return result;
	}
};


}}