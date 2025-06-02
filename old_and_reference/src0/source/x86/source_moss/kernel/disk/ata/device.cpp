#include "device.hpp"

#include "../../kernel.hpp"

#include "../../../mossc/cstring"


namespace MOSS { namespace Disk { namespace ATA {


static void fix_ide_string(char* data, int len) {
	char c, *p=data, *end=data+(len&~1);

	//Swap characters.
	while (p != end) {
		c = *p;
		*p = *(p + 1);
		*(p + 1) = c;
		p += 2;
	}

	//Make sure we have a null byte at the end and wipe out trailing garbage.
	p = end - 1;
	*p-- = '\0';
	while (p-- != data) {
		c = *p;
		if (c>32 && c<127) break;
		*p = '\0';
	}
}


Device::Device(Bus* bus, int index) : _bus(bus), index(index) {}

void Device::fill_information(void) {
	uint16_t buffer[256];
	if (!_bus->command_identify(index, reinterpret_cast<uint8_t*>(buffer))) {
		valid = false;
		return;
	}
	valid = true;

	//Stuff.  See 8.13.9.  TODO: more?
	supports_dma = (buffer[49]>>8) & 1;
	supports_lba = (buffer[49]>>9) & 1;

	num_cylinders = static_cast<uint32_t>(buffer[1]);
	num_heads     = static_cast<uint32_t>(buffer[3]);
	num_sectors   = static_cast<uint32_t>(buffer[6]);

	//	Here, we simplified things a bit.  See Annex B for more information.
	if (supports_lba) {
		capacity = static_cast<uint32_t>(buffer[60]);
	} else {
		capacity = num_heads * num_sectors * num_cylinders;
	}

	//	Copy and massage the information that is useful to us.
	MOSSC::memcpy(   model, buffer+27, 40); fix_ide_string(   model, 40);
	MOSSC::memcpy(  serial, buffer+10, 20); fix_ide_string(  serial, 20);
	MOSSC::memcpy(firmware, buffer+23,  8); fix_ide_string(firmware,  8);

	//update_control_register();
}

/*void Device::update_control_register(void) const {
	//Drive must be current to accept changes to nIEN.  Also, it makes sense.
	assert_term(_bus->current==this,"This drive is not the bus's current drive!");

	//http://wiki.osdev.org/ATA_PIO_Mode#Device_Control_Register_.2F_Alternate_Status
	//	Bit 1 is "nIEN" (0=use IRQs, 1=no interrupts)
	//	Bit 2 is "SRST" (1=do a software reset on all drives on a bus)
	//	Bit 7 is  "HOB" (1=read back the High Order Byte of the last LBA48 value sent to an IO port)

	//By default, drives seem to have interrupts enabled.
	bool interrupts_enabled = true;
	uint8_t control = !interrupts_enabled;
	IO::send<uint8_t>(_bus->_io_base+0x0206,control);
}*/

void Device::print(int level) const {
	kernel->write_sys(level,"Supports:");
	if (supports_dma) kernel->write(" <DMA>");
	if (supports_lba) kernel->write(" <LBA>");
	kernel->write("\n");

	kernel->write_sys(level,"Geometry (C,H,S, capacity): %d,%d,%d, %d\n", num_cylinders,num_heads,num_sectors, capacity);

	kernel->write_sys(level,"Model:    \"%s\"\n",    model);
	kernel->write_sys(level,"Serial:   \"%s\"\n",   serial);
	kernel->write_sys(level,"Firmware: \"%s\"\n", firmware);
}


}}}
