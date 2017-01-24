#pragma once

#include "../../../includes.hpp"

#include "rect.hpp"


namespace MOSST {
	template <typename type> class LinkedList;
}
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


class ComponentBase {
	public:
		ComponentBase*const parent;

		MOSST::LinkedList<ComponentBase*>* children;

		Rect rect_component;
		Rect rect_client;

		bool alive;

	protected:
		ComponentBase(ComponentBase* parent, const Rect& rect_component,const Rect& rect_client=Rect(0,0,0,0));
	public:
		virtual ~ComponentBase(void);

		virtual bool handle_mouse(const Input::Mouse::EventMouseMove& event);
		virtual bool handle_mouse(const Input::Mouse::EventMouseClick& event);
		virtual bool handle_mouse(const Input::Mouse::EventMouseUnclick& event);

		Rect get_client_rect_world(void);
		Rect get_component_rect_world(void);

		virtual void draw(VESA::Framebuffer*/* framebuffer*/) {}
};


}}}
