#include "boot/multiboot.h"

#include "../stdlib/stdlib.h" //TODO: remove

#include "graphics/vesa.h"

#include "input/devices/controller_ps2.h"

#include "interrupt/idt.h"
#include "interrupt/misc.h"
#include "interrupt/pic.h"

#include "memory/gdt.h"
#include "memory/simple.h"

#include "text_mode_terminal.h"

#include "kernel.h"


namespace MOSS { namespace Kernel {


extern "C" void __cxa_pure_virtual(void) {
	//http://wiki.osdev.org/C%2B%2B
	//If, during runtime, the kernel detects that a call to a pure virtual function couldn't be made, it calls the above
	//function.  This function should actually never be called, because without corruption/undefined behaviour, it is not
	//possible to instantiate a class that doesn't define all pure virtual functions.
	ASSERT(false,"__cxa_pure_virtual got called somehow!");
}

extern "C" void kernel_entry(unsigned long magic, unsigned long addr) {
	//The text mode terminal
	Terminal::TextModeTerminal terminal2;
	terminal = &terminal2;
	terminal2.set_color_text(Terminal::TextModeTerminal::COLOR_RED);

	//Check boot went okay
	Boot::multiboot_info_t* mbi = (Boot::multiboot_info_t*)(addr);
	ASSERT(magic==MULTIBOOT_BOOTLOADER_MAGIC,"Invalid multiboot magic number!\n");
	ASSERT(mbi->flags&(1<<0),"Invalid GRUB memory flag!\n");
	ASSERT(mbi->flags&(1<<6),"Invalid GRUB memory map!\n");

	//Setup dynamic memory (some of the setup requires it, i.e. PS/2, so do it first)
	Memory::MemoryManager memory2(mbi);
	memory = &memory2;

	//The GRUB documentation http://www.gnu.org/software/grub/manual/multiboot/multiboot.html implies
	//	that interrupts are already disabled.  That's good, because the following allows us to deal with
	//	them for the first time.
	//terminal->write("Forcing disable hardware interrupts\n");
	//MOSS::Interrupts::disable_hw_int();

	//Setup memory segments
	terminal->write("Loading GDT\n");
	MOSS::Memory::load_gdt();
	terminal->write("Reloading segments\n\n");
	MOSS::Memory::reload_segments();

	//Setup interrupts
	terminal->write("Loading IDT\n");
	MOSS::Interrupts::load_idt();
	terminal->write("Remapping PIC\n\n");
	MOSS::Interrupts::PIC::remap(32,40);

	//Setup PS/2 devices
	terminal->write("Setting up PS/2 controller\n\n");
	Input::Devices::ControllerPS2 controller2;
	controller = &controller2;

	//Enable hardware interrupts
	terminal->write("Enabling hardware interrupts\n");
	MOSS::Interrupts::enable_hw_int();

	/*terminal->write("Test firing . . .\n");
	MOSS::Interrupts::fire_int13h();
	terminal->write("fired!\n");*/

	//The VESA controller needs access to loawer memory (less than 1MiB) load information.  Therefore, it
	//needs to come after all of the information we want is out of the bootloader.
	Graphics::VESA::Controller graphics2;
	graphics = &graphics2;
	/*for (int i=0;i<graphics->numof_modes;++i) {
		graphics->modes[i]->print(terminal);
		delay(1000);
	}*/
	//Graphics::VESA::Mode* mode = graphics->get_mode_closest( 800,600,32);
	Graphics::VESA::Mode* mode = graphics->get_mode_closest(1024,768,32);
	//Graphics::VESA::Mode* mode = graphics->get_mode_closest(1152,864,32);
	graphics->set_mode(mode);
	//graphics->set_pixel(5,5, 255,0,0,255);

	kernel_main();
}


}}