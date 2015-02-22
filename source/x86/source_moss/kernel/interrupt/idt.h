#pragma once
#include "../../includes.h"


namespace MOSS { namespace Interrupts {


extern "C" void idt_lidt(uint32_t base, size_t limit); //ASM routine

//Note that this function ought to be called with hardware interrupts disabled; if an interrupt fires while in progress . . .
void load_idt(void);


}}