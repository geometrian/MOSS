#pragma once


namespace MOSS { namespace Interrupts {


void disable_hw_int(void) {
	asm("cli");
}
void enable_hw_int(void) {
	asm("sti");
}

/*void fire(int n) {
	//Adapted from http://www.brokenthorn.com/Resources/OSDev15.html
	//Self-modifying code!
	asm(
	"	movb  al, [%0]"
	"	movb  [genint+1], al"
	"	jmp   genint"
	"genint:"
	"	int   0"
	: //out
	:"n"(n) //in
	:"%al" //clobbered
	);
}*/
void fire_int13h(void) {
	asm("int  $0x13");
}


}}