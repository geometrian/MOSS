#pragma once

#include "../../../includes.h"

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

class Manager final {
	private:
		MOSST::LinkedList<Frame*> _frames;

		Mouse* _mouse;

		class _Image final { public:
			int const w, h;
			uint8_t* data;

			_Image(int w,int h);
			~_Image(void);
		};
		_Image* _bg;

	public:
		Manager(void);
		~Manager(void);

		void load_bg(char const* path);

		void update(void);

		void handle_mouse(Input::Mouse::   EventMouseMove const& event);
		void handle_mouse(Input::Mouse::  EventMouseClick const& event);
		void handle_mouse(Input::Mouse::EventMouseUnclick const& event);

		void add_frame(MOSST::String const& title, int x,int y,int w,int h);

		void draw(VESA::Framebuffer* framebuffer) const;
};


}}}
