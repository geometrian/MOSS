#pragma once

#include "../../../mosst/linked_list.h"
#include "../../../mosst/string.h"


namespace MOSS {
	namespace Input { namespace Mouse {
		class EventMouseMove;
		class EventMouseClick;
		class EventMouseUnclick;
	}}
	namespace Graphics {
		namespace VESA {
			class Framebuffer;
		}
namespace GUI {


class Frame;

class Mouse;

class Manager {
	private:
		MOSST::LinkedList<Frame*> frames;

		Mouse* mouse;

	public:
		Manager(void);
		~Manager(void);

		void update(void);

		void handle_mouse(const Input::Mouse::EventMouseMove& event);
		void handle_mouse(const Input::Mouse::  EventMouseClick& event);
		void handle_mouse(const Input::Mouse::EventMouseUnclick& event);

		void add_frame(const MOSST::String& title, int x,int y,int w,int h);

		void draw(VESA::Framebuffer* framebuffer) const;
};


}}}