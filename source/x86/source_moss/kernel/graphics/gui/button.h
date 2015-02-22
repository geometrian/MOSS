#pragma once

#include "component.h"


namespace MOSS { namespace Graphics {
	namespace VESA {
		class Framebuffer;
	}
namespace GUI { namespace Buttons {


class ButtonBase : public ComponentBase {
	public:
		int x,y;

		bool hovering;

	protected:
		ButtonBase(ComponentBase* parent, const Rect& rect_button);
	public:
		virtual ~ButtonBase(void);

		bool handle_mouse(const Input::Mouse::EventMouseMove& event) override;
};
class ButtonToggleBase : public ButtonBase {
	private:
		bool state;

	protected:
		ButtonToggleBase(ComponentBase* parent, const Rect& rect_button, bool initial_state);
	public:
		virtual ~ButtonToggleBase(void);
};
class ButtonSingleBase : public ButtonBase {
	protected:
		bool selected;
	protected:
		ButtonSingleBase(ComponentBase* parent, const Rect& rect_button);
	public:
		virtual ~ButtonSingleBase(void);
};
class ButtonClose : public ButtonSingleBase {
	public:
		ButtonClose(ComponentBase* parent);
		virtual ~ButtonClose(void);

		bool handle_mouse(const Input::Mouse::EventMouseClick& event) override;

		void draw(VESA::Framebuffer* framebuffer) override;
};


}}}}