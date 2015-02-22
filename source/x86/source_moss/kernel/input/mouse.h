#pragma once

#include "../../includes.h"


namespace MOSS { namespace Input { namespace Mouse {


class EventMouseMove final { public:
	const int x, y;
	const int dx, dy;
	EventMouseMove(int x,int y, int dx,int dy) : x(x),y(y), dx(dx),dy(dy) {}
};
class EventMouseClick final { public:
	int index;
	EventMouseClick(int index) : index(index) {}
};
class EventMouseUnclick final { public:
	int index;
	EventMouseUnclick(int index) : index(index) {}
};


}}}