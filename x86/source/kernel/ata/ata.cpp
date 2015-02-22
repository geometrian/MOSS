#include "ata.h"

#include "../io/io.h"


namespace MOSS { namespace ATA {


//Each disk controller chip can have up to two ATA buses/channels ("primary" and "secondary").  Each ATA bus has up to two HDDs
//attached ("master" and "slave").  Each HDD has different IO ports:
//
//	                    IO ports           Device Control Register/Alternate Status Port    IRQ
//	Primary Master      [0x01F0,0x01F7]    0x03F6                                            14
//	Primary Slave       [0x0170,0x0177]    0x0376                                            14
//	Secondary Master    [0x01E8,0x01EF]    0x03E6                                            15
//	Secondary Slave     [0x0168,0x016F]    0x0366                                            15


Channel::Channel(Controller* controller, ChannelType type) : controller(controller), type(type) {
	got_irq = false;

	current = NULL;
	drive0 = new Drive(this,Master);
	drive_select(Master);

	//drive0->set_interrupts(false); //Must come after drive select
}
Channel::~Channel(void) {
	delete drive0;
}

void Channel::handle_irq(void) {
	//ASSERT(false,"HDD got IRQ somehow!");
	_read_sector();
	got_irq = true;
}
void Channel::_read_sector(void) {
	ASSERT(current!=NULL,"Current drive was NULL somehow!");
	Drive::StatusByte status = current->get_status();
	ASSERT(!status.err,"Read sectors got ERR!");

	//Transfer 256 words, a word at a time, into buffer from I/O port 0x01F0. (In assembler, REP INSW works well for this).
	for (int j=0;j<512;j+=2) {
		uint16_t word = IO::recv<uint16_t>(current->drive_iobase);
		irq_buffer[j  ] = (uint8_t)( word&0x00FF    );
		irq_buffer[j+1] = (uint8_t)((word&0xFF00)>>8);
	}
}
void Channel::reset_irq(void) {
	got_irq = false;
}

void Channel::drive_select(DriveType drive) {
	//TODO: is this right?
	switch (drive) {
		case Master:
			current = drive0;
			IO::send<uint8_t>(current->drive_iobase+6,0x40);
			/*switch (type) {
				case   Primary: IO::send<uint8_t>(0x01F6,0x40); break; //I know this is.
				case Secondary: IO::send<uint8_t>(0x0176,0x40); break;
			}*/
			break;
		case Slave:
			current = drive1;
			IO::send<uint8_t>(current->drive_iobase+6,0x50);
			/*switch (type) {
				case   Primary: IO::send<uint8_t>(0x01EE,0x50); break;
				case Secondary: IO::send<uint8_t>(0x016E,0x50); break;
			}*/
			break;
	}
}


Drive::Drive(Channel* channel, Channel::DriveType type) : channel(channel), type(type) {
	ASSERT(sizeof(StatusByte)==1,"StatusByte is the wrong size!");

	switch (type) {
		case Channel::Master:
			switch (channel->type) {
				case   Channel::Primary: drive_iobase=0x01F0; break;
				case Channel::Secondary: drive_iobase=0x0170; break;
			}
			break;
		case Channel::Slave:
			switch (channel->type) {
				case   Channel::Primary: drive_iobase=0x01E8; break;
				case Channel::Secondary: drive_iobase=0x0168; break;
			}
			break;
	}

	//Be default, drives seem to have interrupts enabled, but MOSS keeps them disabled
	//by default.  So, change the drive's state to disable interrupts.
	interrupts = true;

	StatusByte status = get_status();
	ASSERT(!status.err,"Drive status was ERR on startup!");
	ASSERT(!status.bsy,"Drive status was BSY on startup!");
	ASSERT(!status.drq,"Drive status was DRQ on startup!");
}
Drive::~Drive(void) {}

void Drive::set_interrupts(bool enable) {
	if (enable!=interrupts) {
		interrupts = enable;
		update_control_register();
	}
}
void Drive::update_control_register(void) {
	//http://wiki.osdev.org/ATA_PIO_Mode#Device_Control_Register_.2F_Alternate_Status
	//Bit 1 is "nIEN" (0=use IRQs, 1=no interrupts)
	//Bit 2 is "SRST" (1=do a software reset on all drives on a bus)
	//Bit 3 is "HOB" (1=read back the High Order Byte of the last LBA48 value sent to an IO port)

	//Drive must be current to accept changes to nIEN.
	ASSERT(channel->current!=NULL,"Channel does not have a current drive!");
	Channel::DriveType temp = channel->current->type;
	channel->drive_select(type);
	switch (type) {
		case Channel::Master:
			switch (channel->type) {
				case   Channel::Primary: IO::send<uint8_t>(0x03F6,!interrupts); break;
				case Channel::Secondary: IO::send<uint8_t>(0x03E6,!interrupts); break;
			}
			break;
		case Channel::Slave:
			switch (channel->type) {
				case   Channel::Primary: IO::send<uint8_t>(0x0376,!interrupts); break;
				case Channel::Secondary: IO::send<uint8_t>(0x0366,!interrupts); break;
			}
			break;
	}
	channel->drive_select(temp);
}

Drive::StatusByte Drive::get_status(void) const {
	//Reading the device control register port actually gets you the value of the alternate status register instead.
	//This value is always the same as the regular status port (0x1F7 on the primary bus), but reading the alternate
	//status port does not affect interrupts.

	//It's unclear about the 400ns delays.  It's probably not necessary, but it's always done here by looping five times.
	uint8_t byte;
	for (int i=0;i<5;++i) {
		switch (type) {
			case Channel::Master:
				switch (channel->type) {
					case   Channel::Primary: byte=IO::recv<uint8_t>(0x03F6); break;
					case Channel::Secondary: byte=IO::recv<uint8_t>(0x03E6); break;
				}
				break;
			case Channel::Slave:
				switch (channel->type) {
					case   Channel::Primary: byte=IO::recv<uint8_t>(0x0376); break;
					case Channel::Secondary: byte=IO::recv<uint8_t>(0x0366); break;
				}
				break;
		}
	}

	StatusByte result;
	result.byte = byte;

	return result;
}

void Drive::read_sectors(uint64_t lba, uint8_t* data_buffer,unsigned int num_sectors) {
	ASSERT(lba<=562949953421311ull,"LBA is larger than (2^49)-1!"); //Only have 48 bits (equals maximum (2^49)-1 addresses = 512TiB)
	ASSERT(num_sectors<=65536,"Number of sectors was larger than 65536!"); //Can't be unsigned short, since 65536 is valid (we have to pass it as zero though):
	unsigned short num_sectors_arg = num_sectors==65536 ? 0 : num_sectors;

	//http://wiki.osdev.org/ATA_PIO_Mode#x86_Directions

	//Note: the high bytes must be sent first.  Theoretically, there's a latch or something
	//that requires this, flipping between whether the expected byte is high or low?  For
	//performance reasons, also don't write to the same IO port twice in a row.
	IO::send<uint8_t>(drive_iobase+2, (num_sectors_arg&        0x0000FF00)>> 8); //high byte of sector count
	IO::send<uint8_t>(drive_iobase+3, (            lba&0x00000000FF000000)>>24); //byte 4 of LBA
	IO::send<uint8_t>(drive_iobase+4, (            lba&0x000000FF00000000)>>32); //byte 5 of LBA
	IO::send<uint8_t>(drive_iobase+5, (            lba&0x0000FF0000000000)>>40); //byte 6 of LBA
	IO::send<uint8_t>(drive_iobase+2,  num_sectors_arg&        0x000000FF     ); //low byte of sector count
	IO::send<uint8_t>(drive_iobase+3,              lba&0x00000000000000FF     ); //byte 1 of LBA
	IO::send<uint8_t>(drive_iobase+4, (            lba&0x000000000000FF00)>> 8); //byte 2 of LBA
	IO::send<uint8_t>(drive_iobase+5, (            lba&0x0000000000FF0000)>>16); //byte 3 of LBA

	//Send the "READ SECTORS EXT" command (0x24) to port 0x01F7:
	channel->irq_buffer = data_buffer;
	IO::send<uint8_t>(0x01F7, 0x24);

	unsigned int i = 0;
	LOOP:
		#if 1 //IRQ
			while (!channel->got_irq);

			//When got_irq is set, the IRQ handler already read the sector and ACKed the interrupt.
			//Just reset the IRQ and loop back to wait for the next sector.

			channel->reset_irq();
		#else //Poll
			do {
				status = get_status();
			} while (status.bsy || !status.drq);
		#endif

		if (++i<num_sectors) {
			channel->irq_buffer += 512;

			//Loop back to waiting for the next IRQ (or poll again -- see next note) for each successive sector.
			goto LOOP;
		}

	//Note for polling PIO drivers: after transferring the last word of a PIO data block to the data IO port, give the drive a
	//400ns delay to reset its DRQ bit (and possibly set BSY again, while emptying/filling its buffer to/from the drive).
	/*IO::wait();
	IO::wait();
	IO::wait();
	IO::wait();
	IO::wait();*/
	for (int i=0;i<100;++i) {
		IO::wait();
	}
}


Controller::Controller(void) {
	channel0 = new Channel(this,Channel::Primary);
	channel1 = NULL;
}
Controller::~Controller(void) {
	delete channel0;
}

const uint8_t* Controller::read_sector(uint64_t lba) {
	//for (int i=0;i<512;++i) buffer[i]=66; //debug value

	read_sectors(lba, buffer,1);

	return buffer;
}
void Controller::read_sectors(uint64_t lba, uint8_t* data_buffer,unsigned int num_sectors) {
	ASSERT(channel0->current!=NULL,"No HDD current!");
	channel0->current->read_sectors(lba, data_buffer, num_sectors);
}


}}