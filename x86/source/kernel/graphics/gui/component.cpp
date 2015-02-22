#include "component.h"


namespace MOSS { namespace Graphics { namespace GUI {


ComponentBase::ComponentBase(ComponentBase* parent, const Rect& rect_component,const Rect& rect_client/*=Rect(0,0,0,0)*/) : parent(parent), rect_component(rect_component),rect_client(rect_client) {
	children = new MOSST::Vector<ComponentBase*>();
}
ComponentBase::~ComponentBase(void) {
	delete children;
}

Rect ComponentBase::get_client_rect_world(void) {
	if (parent==NULL) {
		return rect_client;
	} else {
		Rect parent_rect = parent->get_client_rect_world();
		Rect result = rect_component;
		result.x += parent_rect.x;
		result.y += parent_rect.y;
		return result; //TODO: cache results for efficiency.  Use observer pattern.
	}
}


}}}