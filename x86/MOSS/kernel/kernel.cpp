#include "stdinc.h"

//Must be first code
void kernel_main(void);
extern "C" void kernel_entry(void) {
	kernel_main();
}

//Important includes
#include "ui/console.h"
#include "gdt_idt/gdt.h"
#include "gdt_idt/idt.h"
#include "interrupt.h"

//Includes not used here, but included so that they are compiled
#include "gdt_idt/isr.h"

void kernel_main(void) {
	//INTERRUPT::disable();
	GDT::gdt_load();
	IDT::idt_load();

	/*INTERRUPT::enable();

	A: goto A;*/

	CONSOLE::Console::draw(0,0,"Welcome to MOSS!");

	//KEYBOARD::setup();
	/*asm(
		"int  $0x3"
	);*/

	CONSOLE::Console console;

	LOOP:
		console.update();
		goto LOOP;

	HANG: goto HANG;
}