#pragma once


namespace MOSS { namespace Graphics { namespace GUI {


class Rect {
	public:
		int x,y, w,h;

	public:
		Rect(void) {}
		Rect(const Rect& other) : x(other.x),y(other.y),w(other.w),h(other.h) {}
		Rect(int x,int y, int w,int h) : x(x),y(y),w(w),h(h) {}
};


}}}