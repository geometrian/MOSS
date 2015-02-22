#pragma once
#include "../../includes.h"


namespace MOSS { namespace Memory {


extern "C" void gdt_lgdt(uint32_t base, size_t limit); //ASM routine

//Note that this function ought to be called with hardware interrupts disabled; if an interrupt fires while in progress . . .
void load_gdt(void);


extern "C" void reload_segments(void); //ASM routine


}}