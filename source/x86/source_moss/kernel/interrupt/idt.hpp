#pragma once

#include "../../includes.hpp"


namespace MOSS { namespace Interrupts {


//Load the interrupt descriptor table.
//	Note: this function ought to be called with hardware interrupts disabled.
void load_idt(void);


}}
