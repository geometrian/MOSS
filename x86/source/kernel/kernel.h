namespace MOSS {
	namespace Graphics {
		namespace GUI {
			class Manager;
		}
		namespace VESA {
			class Controller;
		}
	}
	namespace Input {
		namespace Devices {
			class ControllerPS2;
		}
		namespace Keys {
			class Event;
		}
		namespace Mouse {
			class EventMouseMove;
			class EventMouseClick;
			class EventMouseUnclick;
		}
	}
	namespace Terminal {
		class TextModeTerminal;
	}
	namespace Memory {
		class MemoryManager;
	}
namespace Kernel {


extern Terminal::TextModeTerminal* terminal;
extern Memory::MemoryManager* memory;
extern Input::Devices::ControllerPS2* controller;
extern Graphics::VESA::Controller* graphics;
extern Graphics::GUI::Manager* gui;


void handle_key_down(const Input::Keys::Event& event);
void handle_key_up(const Input::Keys::Event& event);

void handle_mouse_move(const Input::Mouse::EventMouseMove& event);
void handle_mouse_click(const Input::Mouse::EventMouseClick& event);
void handle_mouse_unclick(const Input::Mouse::EventMouseUnclick& event);

void kernel_main(void);


}}