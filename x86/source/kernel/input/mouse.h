#pragma once


namespace MOSS { namespace Input { namespace Mouse {


class EventMove { public:
	int x, y;
	int dx, dy;
	EventMove(int x,int y, int dx,int dy) : x(x),y(y), dx(dx),dy(dy) {}
};


}}}