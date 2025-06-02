#include "rect.hpp"


namespace MOSS { namespace Graphics { namespace GUI {


Rect::Rect(Rect const& other) : x(other.x),y(other.y),w(other.w),h(other.h) {}
Rect::Rect(int x,int y, int w,int h) : x(x),y(y),w(w),h(h) {}

bool Rect::intersects(int x, int y) const {
	if (x<this->x        ) return false;
	if (y<this->y        ) return false;
	if (x>this->x+this->w) return false;
	if (y>this->y+this->h) return false;
	return true;
}


}}}
