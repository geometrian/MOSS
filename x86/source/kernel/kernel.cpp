#include "../includes.h"

#include "boot/multiboot.h"
#include "interrupt/gdt_idt.h"
#include "interrupt/interrupts.h"
#include "memory/simple.h"
#include "vesa.h"
#include "text_mode_terminal.h"


namespace MOSS { namespace Kernel {


Terminal::TextModeTerminal* terminal;

Memory::MemoryManager* memory;


#if !defined(__cplusplus)
	#error "C++ should be used!"
#endif
#if defined(__linux__)
	#error "You are not using a cross-compiler!"
#endif


extern "C" void kernel_main(unsigned long magic, unsigned long addr) {
	Terminal::TextModeTerminal terminal2;
	terminal = &terminal2;
	terminal2.set_color_text(Terminal::TextModeTerminal::COLOR_RED);

	multiboot_info_t* mbi = (multiboot_info_t*)(addr);

	#ifdef MOSS_DEBUG
	moss_assert(magic==MULTIBOOT_BOOTLOADER_MAGIC,"Invalid multiboot magic number!\n");
	moss_assert(mbi->flags&(1<<0),"Invalid GRUB memory flag!\n");
	moss_assert(mbi->flags&(1<<6),"Invalid GRUB memory map!\n");
	#endif

	//The GRUB documentation http://www.gnu.org/software/grub/manual/multiboot/multiboot.html implies
	//	that interrupts are already disabled.  That's good, because the following allows us to deal with
	//	them for the first time.

	terminal->write("Forcing disable interrupts\n");
	MOSS::Interrupts::disable();

	terminal->write("Loading GDT\n");
	MOSS::Interrupts::load_gdt();
	terminal->write("Loading IDT\n");
	MOSS::Interrupts::load_idt();

	terminal->write("Reloading Segments\n");
	MOSS::Interrupts::reload_segments();

	terminal->write("Enabling Interrupts\n");
	MOSS::Interrupts::enable();

	terminal->write("Test firing . . .\n");
	MOSS::Interrupts::fire_int13h();
	terminal->write("fired!\n");

	/*Memory::MemoryManager memory2(mbi);
	memory = &memory2;*/

	//set_vesa_mode(640,400,8);
	//set_vesa_mode(800,600,32);
	//set_vesa_pixel(5,5, 255,0,0,255);

	terminal->write("Hanging . . .\n");
	while (true);
}


}}