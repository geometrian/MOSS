#pragma once


namespace MOSS { namespace Input { namespace Mouse {


class EventMouseMove { public:
	const int x, y;
	const int dx, dy;
	EventMouseMove(int x,int y, int dx,int dy) : x(x),y(y), dx(dx),dy(dy) {}
};
class EventMouseClick { public:
	int index;
	EventMouseClick(int index) : index(index) {}
};
class EventMouseUnclick { public:
	int index;
	EventMouseUnclick(int index) : index(index) {}
};


}}}