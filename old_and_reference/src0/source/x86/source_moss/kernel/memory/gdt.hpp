#pragma once

#include "../../includes.hpp"


namespace MOSS { namespace Memory {


//Load the global descriptor table (GDT), which describes memory layout to the CPU.  See ".cpp"
//	file for more info.  Note: this function ought to be called with hardware interrupts
//	disabled.
void load_gdt(void);


//Loading a selector into a segment register automatically reads the GDT and stores its properties
//	inside the processor itself.
extern "C" void reload_segments(void); //ASM routine


}}
