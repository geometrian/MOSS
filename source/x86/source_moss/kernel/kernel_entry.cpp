#include "kernel_entry.h"

#include "../mossc/_misc.h"

#include "disk/ata/controller.h"
#include "disk/filesystems/ext2/ext2.h"
#include "disk/filesystems/fat/fat.h"
#include "disk/disk.h"

#include "boot/multiboot.h"

#include "graphics/gui/manager.h"
#include "graphics/vesa/controller.h"
#include "graphics/vga/terminal.h"
#include "graphics/font.h" //TODO: remove?

#include "input/devices/controller_ps2.h"

#include "interrupt/idt.h"
#include "interrupt/irq_isr.h"
#include "interrupt/misc.h"
#include "interrupt/pic.h"

#include "memory/gdt.h"
#include "memory/simple.h"

#include "kernel.h"
#include "serial.h"


namespace MOSS {


extern Kernel* kernel;

static void test_mode(Graphics::VGA::Device::Mode mode) {
	kernel->vga->set_mode(mode);

	kernel->terminal->clear();
	kernel->terminal->write_test_pattern_res();
	kernel->vga->crtc.print_timing(4,6);

	//Bochs
	for (size_t i=0;i<1000;++i) for (size_t j=0;j<1000;++j) for (size_t k=0;k<50;++k);
	//VirtualBox
	//for (size_t i=0;i<1000;++i) for (size_t j=0;j<1000;++j) for (size_t k=0;k<5000;++k);
}
extern "C" void kernel_entry(unsigned long magic, unsigned long addr) {
	//Set up serial debug IO port ASAP
	#ifdef MOSS_DEBUG
		Serial::init();
	#endif

	//Allocate the kernel object on the stack
	Kernel kernel2;
	kernel = &kernel2;

	//The VGA Adapter
	Graphics::VGA::Device vga;
	kernel->vga = &vga;

	//The text mode terminal
	Graphics::VGA::Terminal terminal(&vga);
	kernel->terminal = &terminal;

	//Check boot went okay
	Boot::multiboot_info_t* mbi = reinterpret_cast<Boot::multiboot_info_t*>(addr);
	assert_term(magic==MULTIBOOT_BOOTLOADER_MAGIC,"Invalid multiboot magic number!\n");
	assert_term(mbi->flags&(1<<0),"Invalid GRUB memory flag!\n");
	assert_term(mbi->flags&(1<<6),"Invalid GRUB memory map!\n");

	//Setup dynamic memory (some of the setup, including printing, requires it, so do it first)
	Memory::MemoryManager memory(mbi);
	kernel->memory = &memory;

	//Set high resolution
	vga.set_use_font(Graphics::Font::font8x8);
	vga.set_mode(Graphics::VGA::Device::Mode::Text128x80);

	kernel->terminal->clear();
	//kernel->terminal->write_test_pattern_res();
	//kernel->vga->crtc.print_timing(4,6);

	//The GRUB documentation http://www.gnu.org/software/grub/manual/multiboot/multiboot.html implies
	//	that interrupts are already disabled.  That's good, because the following allows us to deal with
	//	them for the first time.
	//terminal.write("Forcing disable hardware interrupts\n");
	//MOSS::Interrupts::disable_hw_int();

	terminal.set_color_text(Graphics::VGA::Terminal::Color::GREEN); kernel->write(
		"         _    _ "  "  ____  "   "  ____ "   "  ____ "   "\n"
		"       /| \\  / |" " /  _ \\ "  " /___/ "   " /___/ "   "\n"
		"       ||  \\/  |" "|| / \\ |"  "|\\____ "  "|\\____ "  "\n"
		"       || |\\/| |" "|| \\_/ |"  " \\____\\" " \\____\\" "\n"
		"       ||_| ||_|"  "\\ \\___/ " " _____/"   " _____/"   "\n"
		"       /_/  /_/ "  " \\___/  "  "/____/ "   "/____/ "   "\n"
		"                "  "        "   "       "   "       "   "\n"
	); terminal.set_color_text(Graphics::VGA::Terminal::Color::MAGENTA); kernel->write(
		"  The Minimal Operating System that Sucks\n"
	); terminal.set_color_text(Graphics::VGA::Terminal::Color::PURPLE); kernel->write(
		"    by Ian Mallett\n\n"
	);

	//test_mode(Graphics::VGA::Device::Mode::Text80x25);
	//test_mode(Graphics::VGA::Device::Mode::Text80x30);
	//test_mode(Graphics::VGA::Device::Mode::Text80x50);
	//test_mode(Graphics::VGA::Device::Mode::Text80x60);
	//test_mode(Graphics::VGA::Device::Mode::Text128x48);

	//test_mode(Graphics::VGA::Device::Mode::Text128x80);
	//test_mode(Graphics::VGA::Device::Mode::Text132x25);
	//test_mode(Graphics::VGA::Device::Mode::Text132x43);
	//test_mode(Graphics::VGA::Device::Mode::Text132x50);
	//test_mode(Graphics::VGA::Device::Mode::Text132x60);

	//terminal.interface.crtc.set_mode(Graphics::VGA::CathodeRayTubeController::Mode::text132x60);

	//terminal.interface.set_use_font(Graphics::Font::font8x8);
	//terminal.interface.dump_registers();
	//terminal.interface.set_use_font(Graphics::Font::font8x8);
	//terminal.interface.crtc.set_mode(Graphics::VGA::CathodeRayTubeController::Mode::text128x80);
	//terminal.interface.dump_registers();

	#define MSG0(MESSAGE) kernel->write_sys(0,MESSAGE)
	#define MSG1(MESSAGE) kernel->write_sys(1,MESSAGE)
	#define MSG2(MESSAGE) kernel->write_sys(2,MESSAGE)

	MSG0("Booting . . .\n");
	#if 1 //Set up memory segments
		MSG1("Setting up memory segments:\n");
		MSG2("Loading GDT\n");
		MOSS::Memory::load_gdt();
		MSG2("Reloading segments (to commit GDT)\n\n");
		MOSS::Memory::reload_segments();
	#endif
	#if 1 //Set up interrupts
		MSG1("Setting up interrupts:\n");
		MSG2("Loading IDT\n");
		MOSS::Interrupts::load_idt();
		MSG2("Setting up IRQs\n");
		MOSS::Interrupts::init_irqs();
		MSG2("Remapping PIC\n\n");
		MOSS::Interrupts::PIC::remap(32,40);
	#endif
	#if 0 //Set up PS/2 devices
		MSG1("Setting up PS/2 Devices:\n");
		MSG2("Setting up PS/2 controller\n");
		Input::Devices::ControllerPS2 controller_ps2;
		kernel->controller_ps2 = &controller_ps2;
	#endif
	#if 1 //Set up ATA and hard disk
		MSG1("Setting up ATA controller:\n");
		Disk::ATA::Controller controller_ata;
		kernel->controller_ata = &controller_ata;
		controller_ata.print(2);
	#endif
	#if 1 //Set up FPU
		//TODO: assumes it exists and is on-board the CPU
		MSG1("Setting up FPU:\n\n");
		__asm__ __volatile__("fninit");
	#endif
	#if 1 //Enable hardware interrupts
		MSG1("Enabling hardware interrupts:\n");
		MOSS::Interrupts::enable_hw_int();
		MSG1("\n");
	#endif
	#if 1 //Set up HDD representation (note must come after ATA and enable interrupts)
		MSG1("Setting up hard disk drive:\n");
		Disk::HardDiskDrive disk(&controller_ata, 0,0);
		kernel->disk = &disk;
		disk.print(2);
		MSG1("\n");
	#endif
	#if 1 //Set up file system representation (note must come after ATA, enable interrupts, and HDD representation)
		MSG1("Setting up file system:\n");
		Disk::FileSystem::FileSystemFAT filesystem(disk.partitions[0]);
		kernel->filesystem = &filesystem;
		//kernel->filesystem->print();

		/*Disk::FileSystem::ObjectFileBase* file = filesystem.open("/files/folder_a/folder_ab/file_aba.txt");
		MOSST::Vector<uint8_t>* data = file->get_new_data();
		delete data;*/
	#endif

	//kernel->terminal->interface.dump_registers();

	kernel->write("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789`~!@#$%%^&*()-_=+\\|");
	while (true);

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
	//	needs to come after all of the information we want is out of the bootloader.
	Graphics::VESA::Controller graphics;
	kernel->graphics = &graphics;

	//Set up the GUI manager
	Graphics::GUI::Manager gui;
	kernel->gui = &gui;

	//kernel->write("Loading BG\n");
	gui.load_bg("/files/sunspirenight.ppm");
	//kernel->write("Success!\n");
	//while (1);

	#if 0
		uint8_t buffer[512];
		kernel->write("Reading bootsector from HDD . . .\n");
		kernel->controller_ata->read_sectors(buffer, 0,1);
		kernel->write("Doing it again . . .\n");
		kernel->controller_ata->read_sectors(buffer, 0,1);
		#if 1
			kernel->write("Outputting data . . .\n");
			//for (int i=0;i<512;++i) kernel->write("%X ",static_cast<int>(buffer[i]));
			//for (int i=0;i<512;++i) kernel->write("%d ",static_cast<int>(buffer[i]));
			kernel->write("Bytes at index 510, 511: 0x%X 0x%X\n",buffer[510],buffer[511]); //For MBR should be 0x55 0xAA
		#endif
		kernel->write("Complete!\n"); while (true);
	#endif

	kernel->init();
	kernel->main();
}


}
