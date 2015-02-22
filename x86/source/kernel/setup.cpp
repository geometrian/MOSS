#include "boot/multiboot.h"

#include "../mossc/_misc.h"

#include "ata/ata.h"

#include "graphics/gui/manager.h"
#include "graphics/vesa/controller.h"

#include "input/devices/controller_ps2.h"

#include "interrupt/idt.h"
#include "interrupt/isr.h"
#include "interrupt/misc.h"
#include "interrupt/pic.h"

#include "memory/gdt.h"
#include "memory/simple.h"

#include "text_mode_terminal.h"

#include "kernel.h"


namespace MOSS {


extern "C" void __cxa_pure_virtual(void) {
	//http://wiki.osdev.org/C%2B%2B
	//If, during runtime, the kernel detects that a call to a pure virtual function couldn't be made, it calls the above
	//function.  This function should actually never be called, because without corruption/undefined behavior, it is not
	//possible to instantiate a class that doesn't define all pure virtual functions.
	ASSERT(false,"__cxa_pure_virtual got called somehow!");
}


extern Kernel* kernel;

extern "C" void kernel_entry(unsigned long magic, unsigned long addr) {
	//Allocate the kernel on the stack
	Kernel kernel2;
	kernel = &kernel2;

	//The text mode terminal
	Terminal::TextModeTerminal terminal;
	kernel->terminal = &terminal;
	terminal.set_color_text(Terminal::TextModeTerminal::COLOR_RED);

	//Check boot went okay
	Boot::multiboot_info_t* mbi = (Boot::multiboot_info_t*)(addr);
	ASSERT(magic==MULTIBOOT_BOOTLOADER_MAGIC,"Invalid multiboot magic number!\n");
	ASSERT(mbi->flags&(1<<0),"Invalid GRUB memory flag!\n");
	ASSERT(mbi->flags&(1<<6),"Invalid GRUB memory map!\n");

	//Setup dynamic memory (some of the setup requires it, i.e. PS/2, so do it first)
	Memory::MemoryManager memory(mbi);
	kernel->memory = &memory;

	//The GRUB documentation http://www.gnu.org/software/grub/manual/multiboot/multiboot.html implies
	//	that interrupts are already disabled.  That's good, because the following allows us to deal with
	//	them for the first time.
	//terminal->write("Forcing disable hardware interrupts\n");
	//MOSS::Interrupts::disable_hw_int();

	//Setup memory segments
	kernel->write("Loading GDT\n");
	MOSS::Memory::load_gdt();
	kernel->write("Reloading segments\n\n");
	MOSS::Memory::reload_segments();

	//Setup interrupts
	kernel->write("Loading IDT\n");
	MOSS::Interrupts::load_idt();
	kernel->write("Setting up IRQs\n");
	MOSS::Interrupts::init_irqs();
	kernel->write("Remapping PIC\n\n");
	MOSS::Interrupts::PIC::remap(32,40);

	//Setup PS/2 devices
	kernel->write("Setting up PS/2 controller\n\n");
	Input::Devices::ControllerPS2 controller_ps2;
	kernel->controller_ps2 = &controller_ps2;

	//Setup ATA
	kernel->write("Setting up ATA controller\n\n");
	ATA::Controller controller_ata;
	kernel->controller_ata = &controller_ata;

	//Setup FPU (TODO: assumes it exists and is on-board the CPU).
	kernel->write("Setting up FPU\n\n");
	__asm__ __volatile__("fninit");

	//Enable hardware interrupts
	kernel->write("Enabling hardware interrupts\n");
	MOSS::Interrupts::enable_hw_int();

	/*kernel->write("float a\n");
	float a = 1.0f;
	kernel->write("float b\n");
	float b = 255.0f;
	kernel->write("float c\n");
	float c = a / b;
	kernel->write("float success!\n"); while (true);*/

	/*terminal->write("Test firing . . .\n");
	MOSS::Interrupts::fire_int13h();
	terminal->write("fired!\n");*/

	//The VESA controller needs access to lower memory (less than 1MiB) load information.  Therefore, it
	//needs to come after all of the information we want is out of the bootloader.
	Graphics::VESA::Controller graphics;
	kernel->graphics = &graphics;

	//Set up the GUI manager
	Graphics::GUI::Manager gui;
	kernel->gui = &gui;

	/*kernel->write("Reading bootsector from HDD\n");
	const uint8_t* data = kernel->controller_ata->read_sector(0);
	kernel->write("Writing data . . .\n");
	for (int i=0;i<512;++i) {
		kernel->write("%p ",data[i]);
	}
	//kernel->write("Bytes at index 510, 511: %u %u\n",data[510],data[511]); //For MBR should be 0x55 0xAA
	kernel->write("Complete!\n");*/

	kernel->init();
	kernel->main();
}


}