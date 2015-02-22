#pragma once


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

	static Color blend(const Color& foreground, const Color& background) {
		//#define ROUND(X) ((unsigned char)(X+0.5f))

		Color result;
		float alpha = (float)(foreground.a) / 255.0f;
		float alpha1 = 1.0f - alpha;
		result.r = (unsigned char)(foreground.r*alpha + background.r*alpha1);
		result.g = (unsigned char)(foreground.g*alpha + background.g*alpha1);
		result.b = (unsigned char)(foreground.b*alpha + background.b*alpha1);
		result.a = 255u;

		return result;
	}
};


}}