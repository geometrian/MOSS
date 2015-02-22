#pragma once


namespace MOSS { namespace Graphics { namespace GUI {


class Rect {
	public:
		int x,y, w,h;

	public:
		Rect(void);
		Rect(const Rect& other);
		Rect(int x,int y, int w,int h);

		bool intersects(int x, int y) const;
};


}}}