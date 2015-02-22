namespace MOSS {
	namespace Input {
		namespace Devices {
			class ControllerPS2;
		}
		namespace Keys {
			class Event;
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


void handle_key_down(Input::Keys::Event& event);
void handle_key_up(Input::Keys::Event& event);

void kernel_main(void);


}}