#pragma once

#include "../../../includes.hpp"


namespace MOSS { namespace Graphics { namespace GUI {


class Rect final {
	public:
		int x,y, w,h;

	public:
		inline Rect(void) = default;
		Rect(Rect const& other);
		Rect(int x,int y, int w,int h);
		inline ~Rect(void) = default;

		bool intersects(int x, int y) const;
};


}}}
