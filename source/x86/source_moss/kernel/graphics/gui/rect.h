#pragma once

#include "../../../includes.h"


namespace MOSS { namespace Graphics { namespace GUI {


class Rect final {
	public:
		int x,y, w,h;

	public:
		inline Rect(void) {}
		Rect(Rect const& other);
		Rect(int x,int y, int w,int h);
		inline ~Rect(void) {}

		bool intersects(int x, int y) const;
};


}}}
