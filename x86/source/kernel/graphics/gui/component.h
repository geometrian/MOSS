#pragma once

#include "rect.h"


namespace MOSST {
	template <typename type> class LinkedList;
}
namespace MOSS {
	namespace Input { namespace Mouse {
		class EventMove;
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

	protected:
		ComponentBase(ComponentBase* parent, const Rect& rect_component,const Rect& rect_client=Rect(0,0,0,0));
	public:
		virtual ~ComponentBase(void);

		virtual bool handle_mouse(const Input::Mouse::EventMove& event);

		Rect get_client_rect_world(void);
		Rect get_component_rect_world(void);

		virtual void draw(VESA::Framebuffer*/* framebuffer*/) {}
};


}}}