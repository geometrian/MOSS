#pragma once

#include "../../includes.hpp"


namespace MOSS { namespace Interrupts { namespace PIC {


//http://wiki.osdev.org/PIC
void send_EOI(uint8_t irq);


//The PIC is preconfigured and enabled at boot time by the BIOS (http://forum.osdev.org/viewtopic.php?f=1&t=26875&p=224967#p224967).  When we enable
//	hardware interrupts ("sti"), hardware interrupts can happen.  The BIOS maps the PIC's interrupts so that IRQs [0,7] map to ISRs [8,15] while IRQs
//	[8,15] map to ISRs [112,119].  These mappings are problematic in protected mode since exceptions and whatnot already use interrupt vectors [0,32]!
//	To fix this, we map the PIC's IRQs [0,15] onto ISRs [32,47], which are offsets for the master and slave of 32 and 40, respectively.
void remap(int offset_master, int offset_slave);


}}}
