#pragma once

#include "../../../includes.hpp"

#include "../../../mosst/string.hpp"

#include "component.hpp"


namespace MOSS {
	namespace Input { namespace Mouse {
		class EventMove;
	}}
	namespace Graphics {
		namespace VESA {
			class Framebuffer;
		}
namespace GUI {
	namespace Buttons {
		class ButtonClose;
	}


class Frame final : public ComponentBase {
	private:
		MOSST::String title;
		bool visible;

		Buttons::ButtonClose* button_close;

	public:
		Frame(ComponentBase* parent, int x,int y, int w,int h);
		~Frame(void);

		void set_position(int x, int y);
		void set_size(int width, int height);

		void set_title(const MOSST::String& title);

		void set_visible(bool visible);

		void draw(VESA::Framebuffer* framebuffer) const;
};


}}}
