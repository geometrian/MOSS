#include "component.h"

#include "../../../mosst/linked_list.h"


namespace MOSS { namespace Graphics { namespace GUI {


ComponentBase::ComponentBase(ComponentBase* parent, const Rect& rect_component,const Rect& rect_client/*=Rect(0,0,0,0)*/) : parent(parent), rect_component(rect_component),rect_client(rect_client) {
	if (parent!=NULL) {
		parent->children->insert_back(this);
	}
	children = new MOSST::LinkedList<ComponentBase*>();
}
ComponentBase::~ComponentBase(void) {
	if (parent!=NULL) {
		//TODO: this algorithm actually has n^2 complexity.
		for (int i=0;i<parent->children->size;++i) {
			if ((*parent->children)[i]==this) {
				parent->children->remove(i);
				break;
			}
		}
	}
	delete children;
}

bool ComponentBase::handle_mouse(const Input::Mouse::EventMove& event) {
	for (int i=0;i<children->size;++i) {
		if ((*children)[i]->handle_mouse(event)) return true;
	}
	return false;
}

Rect ComponentBase::get_client_rect_world(void) {
	if (parent==NULL) {
		return rect_client;
	} else {
		Rect parent_rect = parent->get_client_rect_world();
		Rect result = rect_client;
		result.x += parent_rect.x;
		result.y += parent_rect.y;
		return result; //TODO: cache results for efficiency.  Use observer pattern.
	}
}
Rect ComponentBase::get_component_rect_world(void) {
	if (parent==NULL) {
		return rect_component;
	} else {
		Rect parent_rect = parent->get_client_rect_world();
		Rect result = rect_component;
		result.x += parent_rect.x;
		result.y += parent_rect.y;
		return result; //TODO: cache results for efficiency.  Use observer pattern.
	}
}


}}}