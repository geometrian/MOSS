#pragma once

#include "rect.h"

#include "../../../mosst/vector.h"


namespace MOSST {
	template <typename type> class Vector;
}
namespace MOSS { namespace Graphics {
	namespace VESA {
		class Framebuffer;
	}
namespace GUI {


class ComponentBase {
	public:
		ComponentBase*const parent;

		MOSST::Vector<ComponentBase*>* children;

		Rect rect_component;
		Rect rect_client;

	protected:
		ComponentBase(ComponentBase* parent, const Rect& rect_component,const Rect& rect_client=Rect(0,0,0,0));
	public:
		virtual ~ComponentBase(void);

		Rect get_client_rect_world(void);

		virtual void draw(VESA::Framebuffer*/* framebuffer*/) {}
};


}}}