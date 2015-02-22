#pragma once

#include <stddef.h>


namespace MOSSC {
	void* calloc(size_t num, size_t size);
	void* malloc(size_t size);
	void* realloc(void* ptr, size_t size);
	void free(void* ptr);
} namespace MOSS {
	namespace ATA {
		class Controller;
	}
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


extern "C" void kernel_entry(unsigned long magic, unsigned long addr);

class Kernel;

extern Kernel* kernel;

class Kernel {
	friend void kernel_entry(unsigned long magic, unsigned long addr);
	friend void* MOSSC:: calloc(size_t num, size_t size);
	friend void* MOSSC:: malloc(size_t size);
	friend void* MOSSC::realloc(void* ptr, size_t size);
	friend void  MOSSC::free(void* ptr);
	public:
		Terminal::TextModeTerminal* terminal;

		Graphics::VESA::Controller* graphics;
		Graphics::GUI::Manager* gui;

		Input::Devices::ControllerPS2* controller_ps2;

		ATA::Controller* controller_ata;

	private:
		Memory::MemoryManager* memory;

	public:
		//Allocated on the stack, so these can't do much real processing
		Kernel(void);
		~Kernel(void);

		void handle_key_down(const Input::Keys::Event& event);
		void handle_key_up(const Input::Keys::Event& event);
		void handle_mouse_move(const Input::Mouse::EventMouseMove& event);
		void handle_mouse_click(const Input::Mouse::EventMouseClick& event);
		void handle_mouse_unclick(const Input::Mouse::EventMouseUnclick& event);

		void write(const char* format, ...);

		void init(void);
		void main(void);
};


}