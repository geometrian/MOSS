#pragma once

#include "../../includes.h"


namespace MOSS { namespace Interrupts {


inline void disable_hw_int(void) {
	__asm__ __volatile__("cli");
}
inline void enable_hw_int(void) {
	__asm__ __volatile__("sti");
}

/*inline void fire(int n) {
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
inline void fire_int13h(void) {
	__asm__ __volatile__("int  $0x13");
}


}}
