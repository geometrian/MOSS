#include "drive.h"

#include "../io/io.h"
#include "../kernel.h"

#include "../../mossc/cstring"


namespace MOSS { namespace ATA {


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


Drive::Drive(Bus* bus, int index) : _bus(bus), index(index) {}

void Drive::init(void) {
	assert_term(_bus->current==this,"This drive is not the bus's current drive!");

	//IDENTIFY command
	IO::send<uint8_t>(_bus->_io_base+0x0002,0x00);
	IO::send<uint8_t>(_bus->_io_base+0x0003,0x00);
	IO::send<uint8_t>(_bus->_io_base+0x0004,0x00);
	IO::send<uint8_t>(_bus->_io_base+0x0005,0x00);
	IO::send<uint8_t>(_bus->_io_base+0x0007,0xEC);
	StatusByte status = IO::recv<uint8_t>(_bus->_io_base+0x0007);
	if (status.byte==0x00) {
		//kernel->write("Drive %d failed at A\n",index);
		valid = false;
		return;
	} else {
		//Wait for BSY to clear
		while (status.bsy) status=IO::recv<uint8_t>(_bus->_io_base+0x0007);

		//Because of some ATAPI drives that do not follow the spec., need to check the LBAmid and LBAhi ports to see if they are non-zero.
		//	If so, the drive is not ATA.
		uint8_t  mid = IO::recv<uint8_t>(_bus->_io_base+0x0004);
		uint8_t high = IO::recv<uint8_t>(_bus->_io_base+0x0005);
		if (mid || high) {
			//kernel->write("Drive %d failed at B\n",index);
			valid = false; //Not supported!
			return;
		}

		//Poll one of the status ports until DRQ or ERR sets.
		while (!(status.drq || status.err)) status=IO::recv<uint8_t>(_bus->_io_base+0x0007);

		if (status.err) {
			//kernel->write("Drive %d failed at C\n",index);
			valid = false;
			return;
		}

		//The IDENTIFY command succeded.  Collect the data.
		uint16_t temp[256];
		for (int i=0;i<256;++i) {
			temp[i] = IO::recv<uint16_t>(_bus->_io_base/*+0x0000*/);
		}

		supports_lba = (temp[49] >> 9) & 1;
		supports_dma = (temp[49] >> 8) & 1;

		num_cylinders = static_cast<uint32_t>(temp[1]);
		num_heads     = static_cast<uint32_t>(temp[3]);
		num_sectors   = static_cast<uint32_t>(temp[6]);

		//	Here, we simplified things a bit.  See ATA/ATAPI-4 spec, Annexe B for more information.
		if (supports_lba) {
			capacity = static_cast<uint32_t>(temp[60]);
		} else {
			capacity = num_heads * num_sectors * num_cylinders;
		}

		//	Copy and massage the information that is useful to us.
		MOSSC::memcpy(   model, temp+27, 40); fix_ide_string(   model, 40);
		MOSSC::memcpy(  serial, temp+10, 20); fix_ide_string(  serial, 20);
		MOSSC::memcpy(firmware, temp+23,  8); fix_ide_string(firmware,  8);

		valid = true;
		//kernel->write("Drive %d succeeded!\n",index);
	}

	/*//See ATA/ATAPI-4 specification, section 8.12.5.2 and 9.1.  I also looked at Bochs's BIOS listing
	//	("rombios.c") as the specification does not seem to be respected.
	IO::recv<uint8_t>(
	if (inb(iobase + ATA_NSECTOR) == 0x01 &&
		inb(iobase + ATA_SECTOR) == 0x01) {
		cl = inb(iobase + ATA_LCYL);
		ch = inb(iobase + ATA_HCYL);
		status = inb(iobase + ATA_STATUS);
		if (cl == 0x14 && ch == 0xeb) {
			//This device implements the PACKET Command feature set.
			device->present = TRUE;
			device->atapi = TRUE;
		}
		else if (cl == 0 && ch == 0 && status != 0) {
			//This device does not implement the PACKET Command feature set.
		}
		valid =  true;
	} else {
		valid = false;
		return;
	}



	int i, iobase = _bus->_io_base;
	byte_t status, cl, ch, cmd;
	uint16_t info[256];

	cmd = device->atapi ? ATAPI_IDENTIFY : ATA_IDENTIFY;

	//Send the IDENTIFY (PACKET) DEVICE command.
	outb(iobase + ATA_COMMAND, cmd); udelay(1);

	//See ATA/ATAPI-4 spec, section 9.7
	if (!wait_for_controller(device->controller,
		ATA_STATUS_BSY | ATA_STATUS_DRQ | ATA_STATUS_ERR,
		ATA_STATUS_DRQ, ATA_TIMEOUT)) {
		device->present = FALSE;
		return;
	}*/

	update_control_register();

	status = get_status();
	assert_term(!status.err,"Drive status was ERR on startup!");
	assert_term(!status.bsy,"Drive status was BSY on startup!");
	assert_term(!status.drq,"Drive status was DRQ on startup!");
}

void Drive::select(void) const {
	switch (index) {
		case 0:
			//IO::send<uint8_t>(_bus->_io_base+0x0006,0x40);
			IO::send<uint8_t>(_bus->_io_base+0x0006,0xA0);
			break;
		case 1:
			//IO::send<uint8_t>(_bus->_io_base+0x0006,0x50);
			IO::send<uint8_t>(_bus->_io_base+0x0006,0xB0);
			break;
		default:
			assert_term(false,"Implementation error!");
	}
}

void Drive::read_ready_sector(uint8_t* data) const {
	assert_term(_bus->current==this,"This drive is not the bus's current drive!");

	//Transfer 256 words, a word at a time, into buffer. (In assembler, REP INSW works well for this).
	for (int j=0;j<512;j+=2) {
		uint16_t word = IO::recv<uint16_t>(_bus->_io_base/*+0x0000*/);
		data[j  ] = static_cast<uint8_t>( word&0x00FF    );
		data[j+1] = static_cast<uint8_t>((word&0xFF00)>>8);
	}
}

Drive::StatusByte Drive::get_status(void) const {
	assert_term(_bus->current==this,"This drive is not the bus's current drive!");

	//Note: could read the regular status port (offset 0x0007), but this screws up interrupts
	//	for some preposterous reason.  Instead read the alternate status register (offset 0x0206)
	//	which is exactly the same except it doesn't deliberately shoot itself.
	uint8_t byte = IO::recv<uint8_t>(_bus->_io_base+0x0206);

	StatusByte result;
	result.byte = byte;

	return result;
}

void Drive::update_control_register(void) const {
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
}

void Drive::print(int indent) const {
	for (int i=0;i<indent;++i) kernel->write("  ");
	kernel->write("Supports:");
	if (supports_lba) kernel->write(" <LBA>");
	if (supports_dma) kernel->write(" <DMA>");
	kernel->write("\n");

	for (int i=0;i<indent;++i) kernel->write("  ");
	kernel->write("Geometry (C,H,S, capacity): %d,%d,%d, %d\n", num_cylinders,num_heads,num_sectors, capacity);

	for (int i=0;i<indent;++i) kernel->write("  "); kernel->write("Model:    \"%s\"\n",    model);
	for (int i=0;i<indent;++i) kernel->write("  "); kernel->write("Serial:   \"%s\"\n",   serial);
	for (int i=0;i<indent;++i) kernel->write("  "); kernel->write("Firmware: \"%s\"\n", firmware);
}


}}