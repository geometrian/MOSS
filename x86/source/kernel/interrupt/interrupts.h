#pragma once


namespace MOSS { namespace Interrupts {


void disable(void) {
	asm("cli");
}
void enable(void) {
	asm("sti");
}

void fire_int13h(void) {
	asm("int  $0x13");
}


}}