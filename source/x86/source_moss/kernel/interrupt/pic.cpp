#include "pic.hpp"

#include "../../includes.hpp"

#include "../io/io.hpp"


namespace MOSS { namespace Interrupts { namespace PIC {


#define PIC_MASTER 0x0020 //IO base address for master PIC
#define PIC_SLAVE  0x00A0 //IO base address for slave PIC

#define PIC_MASTER_COMMAND  PIC_MASTER
#define  PIC_SLAVE_COMMAND   PIC_SLAVE
#define PIC_MASTER_DATA    (PIC_MASTER+1)
#define  PIC_SLAVE_DATA    ( PIC_SLAVE+1)

#define PIC_EOI 0x20 //End-of-interrupt command code


void send_EOI(uint8_t irq) {
	//If the IRQ came from the master PIC, it is sufficient to issue this command only to the Master PIC
	//If the IRQ came from the slave PIC, it is necessary to issue the command to both PICs.
	if (irq >= 8) {
		IO::send<uint8_t>(PIC_SLAVE_COMMAND,PIC_EOI);
	}
	IO::send<uint8_t>(PIC_MASTER_COMMAND,PIC_EOI);
}

void remap(int offset_master, int offset_slave) {
	//TODO: what IS all this?
	//See page 10: http://pdos.csail.mit.edu/6.828/2005/readings/hardware/8259A.pdf
	#define ICW1_ICW4      0x01 //ICW4 (not) needed
	#define ICW1_SINGLE    0x02 //Single (cascade) mode
	#define ICW1_INTERVAL4 0x04 //Call address interval 4 (8)
	#define ICW1_LEVEL     0x08 //Level triggered (edge) mode
	#define ICW1_INIT      0x10 //Initialization - required!

	//Save masks
	uint8_t mask1 = IO::recv<uint8_t>(PIC_MASTER_DATA);
	uint8_t mask2 = IO::recv<uint8_t>( PIC_SLAVE_DATA);

	//The initialization sequence is made up of four "initialization words" ("ICWs") (actually bytes).  Pass the first
	//initialization word (ICW1_INIT|ICW1_ICW4=0x11) (a command) that begins the sequence (in cascade mode).  This will
	//cause the PIC to wait for three more ICWs (which are data).  These data ICWs, in order, give the PIC:
	//	1: Its vector offset (the offset into the IDT to use)
	//	2: Its configuration (how wired to master/slave)
	//	3: Additional information about environment
	IO::send<uint8_t>(PIC_MASTER_COMMAND, ICW1_INIT|ICW1_ICW4); IO::wait();
	IO::send<uint8_t>( PIC_SLAVE_COMMAND, ICW1_INIT|ICW1_ICW4); IO::wait();

	//Set offsets
	IO::send<uint8_t>(PIC_MASTER_DATA, offset_master); IO::wait();
	IO::send<uint8_t>( PIC_SLAVE_DATA,  offset_slave); IO::wait();

	//Set configuration
	//	Tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
	//	Tell Slave PIC its cascade identity (0000 0010)
	IO::send<uint8_t>(PIC_MASTER_DATA, 0x04); IO::wait();
	IO::send<uint8_t>( PIC_SLAVE_DATA, 0x02); IO::wait();

	//Set mode:
	//	0x01 is 8086/88 (MCS-80/85) mode
	//	0x02 is auto (normal) EOI
	//	0x08 is buffered mode/slave
	//	0x0C is buffered mode/master
	//	0x10 is "Special fully nested (not)"
	IO::send<uint8_t>(PIC_MASTER_DATA, 0x01); IO::wait();
	IO::send<uint8_t>( PIC_SLAVE_DATA, 0x01); IO::wait();

	//Restore saved masks.
	//	TODO: is io_wait required here too?
	IO::send<uint8_t>(PIC_MASTER_DATA, mask1);
	IO::send<uint8_t>( PIC_SLAVE_DATA, mask2);
}


}}}
