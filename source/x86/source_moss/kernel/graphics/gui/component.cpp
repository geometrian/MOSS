#include "component.h"

#include "../../../mosst/linked_list.h"


namespace MOSS { namespace Graphics { namespace GUI {


ComponentBase::ComponentBase(ComponentBase* parent, const Rect& rect_component,const Rect& rect_client/*=Rect(0,0,0,0)*/) :
	parent(parent),
	rect_component(rect_component),rect_client(rect_client),
	alive(true)
{
	if (parent!=nullptr) {
		parent->children->insert_back(this);
	}
	children = new MOSST::LinkedList<ComponentBase*>();
}
ComponentBase::~ComponentBase(void) {
	if (parent!=nullptr) {
		for (auto iter=parent->children->cbegin(); iter!=parent->children->cend(); ++iter) {
			if (*iter==this) {
				~iter;
				break;
			}
		}
	}
	delete children;
}

bool ComponentBase::handle_mouse(const Input::Mouse::EventMouseMove& event) {
	for (auto iter=parent->children->cbegin(); iter!=parent->children->cend(); ++iter) {
		if ((*iter)->handle_mouse(event)) return true;
	}
	return false;
}
bool ComponentBase::handle_mouse(const Input::Mouse::EventMouseClick& event) {
	for (auto iter=parent->children->cbegin(); iter!=parent->children->cend(); ++iter) {
		if ((*iter)->handle_mouse(event)) return true;
	}
	return false;
}
bool ComponentBase::handle_mouse(const Input::Mouse::EventMouseUnclick& event) {
	for (auto iter=parent->children->cbegin(); iter!=parent->children->cend(); ++iter) {
		if ((*iter)->handle_mouse(event)) return true;
	}
	return false;
}

Rect ComponentBase::get_client_rect_world(void) {
	if (parent==nullptr) {
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
	if (parent==nullptr) {
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
