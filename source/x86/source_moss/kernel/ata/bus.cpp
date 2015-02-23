#include "bus.h"

#include "../io/io.h"

#include "drive.h"


namespace MOSS { namespace ATA {


Bus::Bus(Controller* controller, int index) :
	_controller(controller),
	index(index),
	_io_base(index==0||index==1 ? (index==0 ? 0x01F0 : 0x0170) : (index==2 ? 0x01E8 : 0x0168))
{
	assert_term(index>=0&&index<=3,"Implementation error!");

	//Need to check that this is a real ATA bus.  In modern computers (and by default in Bochs), there
	//	are two: the primary and secondary.  One way to check is to write a value and check if it sticks.
	//	This is terrible.  TODO: better way?
	//Note: an ATA bus can be disabled in Bochs by using "ata[0123]: enabled=0" in ".bochsrc".
	IO::send<uint8_t>(_io_base+0x0002,0xAB);
	if (IO::recv<uint8_t>(_io_base+0x0002)!=0xAB) {
		valid = false;
		return;
	}
	valid = true;

	for (int i=0;i<2;++i) {
		drives[i] = new Drive(this, i);
		drive_select(i);
		drives[i]->init();
		if (!drives[i]->valid) {
			delete drives[i];
			drives[i] = nullptr;
		}
	}
	current = nullptr;
	drive_select(0);
}
Bus::~Bus(void) {
	if (valid) {
		for (int i=0;i<2;++i) {
			if (drives[i]!=nullptr) delete drives[i];
		}
	}
}

void Bus::drive_select(int drive_index) {
	//See ATA/ATAPI-4 specification, section 9.7

	assert_term(drive_index>=0&&drive_index<=1,"Invalid drive index \"%d\"!",drive_index);
	assert_term(drives[drive_index]!=nullptr,"Drive index %d does not exist!",drive_index);
	drives[drive_index]->select();
	current = drives[drive_index];

	//After issuing a drive select, need to wait 400ns.  The ATA specification suggests
	//	reading the status register five times.
	for (int i=0;i<5;++i) {
		current->get_status();
	}

	/*//Reset the bus.  This step is apparently required (although not by the ATA/ATAPI-4
	//	specification) to get the correct device signature after a drive has been selected.
	reset_controller(device->controller);*/
}

//Note: IRQs cannot be interrupted by other IRQs.
void Bus::handle_irq(void) {
	_read_sector_from_current_drive();
}
void Bus::_read_sector_from_current_drive(void) {
	assert_term(current!=nullptr,"Current drive was null somehow!");
	assert_term(!current->get_status().err,"Read sectors got ERR!");

	current->read_ready_sector(_irq_buffer);
	_irq_buffer += 512;
}

void Bus::read_sectors_from_current_drive(uint64_t lba, uint8_t* data_buffer,int num_sectors) {
	_irq_buffer = data_buffer;

	assert_term(lba<=562949953421311ull,"LBA is larger than (2^49)-1!"); //Only have 48 bits (equals maximum (2^49)-1 addresses = 512TiB)
	assert_term(num_sectors<=65536,"Number of sectors was larger than 65536!"); //Can't be unsigned short, since 65536 is valid (we have to pass it as zero though)
	unsigned short num_sectors_arg = num_sectors==65536 ? 0 : num_sectors;

	//http://wiki.osdev.org/ATA_PIO_Mode#x86_Directions

	//Note: the high bytes must be sent first.  Theoretically, there's a latch or something
	//	that requires this, flipping between whether the expected byte is high or low?  For
	//	performance reasons, also don't write to the same IO port twice in a row.
	IO::send<uint8_t>(_io_base+0x0002, (num_sectors_arg&        0x0000FF00)>> 8); //high byte of sector count
	IO::send<uint8_t>(_io_base+0x0003, (            lba&0x00000000FF000000)>>24); //byte 4 of LBA
	IO::send<uint8_t>(_io_base+0x0004, (            lba&0x000000FF00000000)>>32); //byte 5 of LBA
	IO::send<uint8_t>(_io_base+0x0005, (            lba&0x0000FF0000000000)>>40); //byte 6 of LBA
	IO::send<uint8_t>(_io_base+0x0002,  num_sectors_arg&        0x000000FF     ); //low byte of sector count
	IO::send<uint8_t>(_io_base+0x0003,              lba&0x00000000000000FF     ); //byte 1 of LBA
	IO::send<uint8_t>(_io_base+0x0004, (            lba&0x000000000000FF00)>> 8); //byte 2 of LBA
	IO::send<uint8_t>(_io_base+0x0005, (            lba&0x0000000000FF0000)>>16); //byte 3 of LBA

	//Send the "READ SECTORS EXT" command (0x24) to the command port:
	IO::send<uint8_t>(_io_base+0x0007, 0x24);

	//Busy wait for a series of IRQs to read the sector.  Not polling.
	while (_irq_buffer-data_buffer<512*num_sectors);

	//Note for polling PIO drivers: after transferring the last word of a PIO data block to the data IO port, give the drive a
	//400ns delay to reset its DRQ bit (and possibly set BSY again, while emptying/filling its buffer to/from the drive).
	/*IO::wait();
	IO::wait();
	IO::wait();
	IO::wait();
	IO::wait();*/
	/*for (int i=0;i<100;++i) {
		IO::wait();
	}*/
}


}}