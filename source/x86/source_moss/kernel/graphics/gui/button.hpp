#pragma once

#include "../../../includes.hpp"

#include "component.hpp"


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
		inline virtual ~ButtonBase(void) = default;

		virtual bool handle_mouse(const Input::Mouse::EventMouseMove& event) override;
};
class ButtonToggleBase : public ButtonBase {
	private:
		bool state;

	protected:
		ButtonToggleBase(ComponentBase* parent, const Rect& rect_button, bool initial_state);
	public:
		inline virtual ~ButtonToggleBase(void) = default;
};
class ButtonSingleBase : public ButtonBase {
	protected:
		bool selected;
	protected:
		ButtonSingleBase(ComponentBase* parent, const Rect& rect_button);
	public:
		inline virtual ~ButtonSingleBase(void) = default;
};
class ButtonClose : public ButtonSingleBase {
	public:
		ButtonClose(ComponentBase* parent);
		inline virtual ~ButtonClose(void) = default;

		virtual bool handle_mouse(const Input::Mouse::EventMouseClick& event) override;

		virtual void draw(VESA::Framebuffer* framebuffer) override;
};


}}}}
