#pragma once

#include "../includes.hpp"


namespace MOSSC {
	void* calloc(size_t num, size_t size);
	void* malloc(size_t size);
	void* realloc(void* ptr, size_t size);
	void free(void* ptr);
} namespace MOSS {
	namespace Disk {
		namespace ATA {
			class Controller;
		}
		namespace FileSystem {
			class FileSystemBase;
		}
		class HardDiskDrive;
	}
	namespace Graphics {
		namespace GUI {
			class Manager;
		}
		namespace VGA {
			class Device;
			class Terminal;
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
	namespace Interrupts {
		class InterruptState;
	}
	namespace Memory {
		class MemoryManager;
	}
	namespace Process {
		class Scheduler;
	}


extern "C" void kernel_entry(unsigned long magic, unsigned long addr);

class Kernel;

extern Kernel* kernel;

class Kernel final {
	friend void kernel_entry(unsigned long magic, unsigned long addr);
	friend void* MOSSC:: calloc(size_t num, size_t size);
	friend void* MOSSC:: malloc(size_t size);
	friend void* MOSSC::realloc(void* ptr, size_t size);
	friend void  MOSSC::free(void* ptr);
	public:
		Graphics::VGA::Terminal* terminal;

		Graphics::VGA::Device* vga;
		Graphics::VESA::Controller* graphics;
		Graphics::GUI::Manager* gui;

		Input::Devices::ControllerPS2* controller_ps2;

		Disk::ATA::Controller* controller_ata;
		Disk::HardDiskDrive* disk;
		Disk::FileSystem::FileSystemBase* filesystem;

	private:
		Memory::MemoryManager* memory;

		Process::Scheduler* scheduler;

	public:
		//Allocated on the stack, so these can't do much real processing
		Kernel(void);
		~Kernel(void) = default;

		void handle_key_down(Input::Keys::Event const& event);
		void handle_key_up(Input::Keys::Event const& event);

		void handle_mouse_move(Input::Mouse::EventMouseMove const& event);
		void handle_mouse_click(Input::Mouse::EventMouseClick const& event);
		void handle_mouse_unclick(Input::Mouse::EventMouseUnclick const& event);

		void handle_timer(Interrupts::InterruptState const* state);

		void write(char const* format,...);
		void write_sys(int level, char const* format,...);

		void init(void);
		void main(void);
};


}
