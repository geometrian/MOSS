#include "bus.hpp"

#include "../../io/io.hpp"
#include "../../kernel.hpp"

#include "device.hpp"


namespace MOSS { namespace Disk { namespace ATA {


void Bus::_StatusByte::print(void) const {
	kernel->write("[ERR %d, DRQ %d, SRV %d, DF %d, RDY %d, BSY %d]",err,drq,srv,df,drdy,bsy);
}


Bus::Bus(Controller* controller, int index) :
	_controller(controller),
	index(index),
	_io_base(index==0||index==1 ? (index==0 ? 0x01F0 : 0x0170) : (index==2 ? 0x01E8 : 0x0168))
{
	assert_term(index>=0&&index<=3,"Implementation error!");

	//Need to check that this is a real ATA bus.  In modern computers (and by default in Bochs), there
	//	are two: the primary and secondary.  Unfortunately, I can't find a way to do just this.  One
	//	suggestion is to just write a value (to the 0th device) and check if it sticks; this actually
	//	detects that a device exists on the bus, not just that the bus exists.  Maybe there's a better
	//	way that's more specific.
	//TODO: better way?
	//Note: an ATA bus can be disabled in Bochs by using "ata[0123]: enabled=0" in ".bochsrc".
	_write_uint8(Write::RegisterOffset::NUM_SECTORS,0xAB);
	if (_read_uint8(Read::RegisterOffset::NUM_SECTORS)!=0xAB) {
		valid = false;
		return;
	}
	valid = true;

	for (int i=0;i<2;++i) {
		devices[i] = new Device(this, i);
	}
	current = nullptr;

	for (int i=0;i<2;++i) {
		devices[i]->fill_information();

		if (!devices[i]->valid) {
			delete devices[i];
			devices[i] = nullptr;
		}
	}

	//bool interrupts_enabled = true;
	//uint8_t control = !interrupts_enabled;
	//_select_device(0);
	//_write_uint8(Write::DEVICE_CONTROL,control);
}
Bus::~Bus(void) {
	if (valid) {
		for (int i=0;i<2;++i) {
			if (devices[i]!=nullptr) delete devices[i];
		}
	}
}

void Bus::_transfer_block_into(uint8_t* data) const {
	//Transfer 256 words, a word at a time, into buffer. (In assembler, REP INSW works well for this).
	#if 0
		for (int j=0;j<512;j+=2) {
			uint16_t word = _read_uint16(Read::RegisterOffset::DATA);
			data[j  ] = static_cast<uint8_t>( word&0x00FF    );
			data[j+1] = static_cast<uint8_t>((word&0xFF00)>>8);
		}
	#else
		uint16_t* ptr = reinterpret_cast<uint16_t*>(data);
		for (int i=0;i<256;++i) {
			ptr[i] = _read_uint16(Read::RegisterOffset::DATA);
		}
	#endif
}

//Protocols (assuming nIEN is cleared and standard operation):
//	PIO data in command protocol (9.7; logical pg. [224,226]) (transfer of data from device to host):
//		1. Select device (9.6; logical pg. 223).
//		2. Write any required command parameters to the Features, Sector Count, Sector Number, Cylinder
//		   High, Cylinder Low, and Device/Head registers.
//		3. Write command code to the Command register; starts execution.
//		4. An interrupt happens.  In the handler, should read the Status register (after 400ns?).  If
//		   an error happened, we're done, and we can start over with a new command.  If not, proceed
//		   to 5.
//		5. Read the data register until the block transfer is complete.  If there's more data for the
//		   command, goto 4.  If not, read the alternate status register (ignore result) and then read
//		   the Status Register.
//	PIO data out command protocol (9.8; logical pg. [227,230]) (transfer of data from host to device):
//		1. Select device (9.6; logical pg. 223).
//		2. Write any required command parameters to the Features, Sector Count, Sector Number, Cylinder
//		   High, Cylinder Low, and Device/Head registers.
//		3. Write command code to the Command register; starts execution.
//		4. If an error happens, a terminal interrupt happens (goto 9).  If there is no error, proceed
//		   to 5.
//		5. Wait 400ns and then poll the Status or the Alternate Status register until BSY is clear and
//		   DRQ is set.
//		6. Write the Data register.  Repeat until the block transfer is complete.
//		7. Wait for an interrupt.  If that is all the data for the command, goto 9.  Else 8.
//		8. Intermediate interrupt handler: read the Status register, then goto 6.
//		9. Terminal interrupt handler: should read Status register, and then can start over with a new
//		   command.
//	Device reset protocol (9.4; logical pg. 219) (reset device)
//		1. Write DEVICE RESET in Command register
//		2. ???
//	Non-data command protocol (9.9; logical pg. [231,232]) (execution of commands with no data transfer)
//		1. Select device (9.6; logical pg. 223).
//		2. Write any required command parameters to the Features, Sector Count, Sector Number, Cylinder
//		   High, Cylinder Low, and Device/Head registers.
//		3. Write command code to the Command register; starts execution.
//		4. Wait for interrupt.  When received, read Status register.
void Bus::_select_device(int device_index) {
	//From ATA/ATAPI-4 9.6:
	//	"Before issuing any command to a device except the DEVICE RESET command, the host shall insure that
	//	the selected device is no longer busy, select the desired device, and insure that it is ready to
	//	accept a command."

	assert_term(device_index>=0&&device_index<=1,"Invalid device index \"%d\"!",device_index);
	assert_term(devices[device_index]!=nullptr,"Drive index %d does not exist!",device_index);

	//ATA/ATAPI-4 9.6 figure 11 (logical pg. 223)
	_StatusByte status;

	do {
		status = _read_uint8(Read::RegisterOffset::STATUS_REGULAR);
	} while (!(status.bsy==0 && status.drq==0));

	//	It is unclear whether this value bogusly depends on certain other bits to be set.  I'm going to
	//		assume not and hope for the best.
	uint8_t value = device_index << 4;
	_write_uint8(Write::RegisterOffset::DEVICE_HEAD,value);
	//	Wait >= 400ns
	for (int i=0;i<5;++i) IO::wait();

	do {
		status = _read_uint8(Read::RegisterOffset::STATUS_REGULAR);
	} while (!(status.bsy==0 && status.drq==0));

	/*#ifdef MOSS_DEBUG
	if ( status.bsy || !status.drdy || status.drq || status.err) {
		status.print(); kernel->write("\n");
		assert_term(false,"Unexpected status!");
	}
	#endif*/

	current = devices[device_index];
}
void Bus::_select_device_as_necessary(int device_index) {
	//TODO: cache which device index is currently selected!
	_select_device(device_index);
}
void Bus::_proto_pio_data_in(uint8_t const* end) {
	//PIO data in protocol steps 4 and 5.

	//Wait for interrupts to finish.  Termination when the command's buffer is filled or an error occurs.
	while ( _current_command_buffer!=nullptr && _current_command_buffer<end );

	_read_uint8(Read::RegisterOffset::STATUS_ALTERNATE);
	_read_uint8(Read::RegisterOffset::  STATUS_REGULAR);
}
/*void Bus::command_resetdevice(int device_index) {
	//ATA/ATAPI-4 8.7 (logical pg. 67)
	//	  7 |  6 |   5 |   4 |  3 |  2 |  1 |  0
	//	obs | na | obs | DEV | na | na | na | na
	//	Protocol: device reset (section 9.4)

	//0x08
	assert_term(false,"Not implemented!");
}*/
/*void Bus::command_flushcache(int device_index) {
	//ATA/ATAPI-4 8.10 (logical pg. 73)
	//	  7 |  6 |   5 |   4 |  3 |  2 |  1 |  0
	//	obs | na | obs | DEV | na | na | na | na
	//	Protocol: non-data
	_select_device_as_necessary(device_index);

	//Write::RegisterOffset::FEATURES_ERRORINFO not applicable
	//Write::RegisterOffset::       NUM_SECTORS not applicable
	//Write::RegisterOffset::           LBA_LOW not applicable
	//Write::RegisterOffset::           LBA_MID not applicable
	//Write::RegisterOffset::          LBA_HIGH not applicable
	_write_uint8(Write::RegisterOffset::DEVICE_HEAD,device_index<<4);
	_current_command                               = 0xE7;
	_write_uint8(Write::RegisterOffset::    COMMAND, 0xE7);

	assert_term(false,"Not implemented!");
}*/
bool Bus::command_identify(int device_index, uint8_t data_buffer[512]) {
	//The ATA specification must be lying.  The code below, implemented painstakingly from the documentation,
	//	fails because the standard-required interrupt never comes.  Instead, do what everyone else does.  See
	//	also http://forum.osdev.org/viewtopic.php?f=1&t=29074
	#if 0
		kernel->write("Selecting device %d on bus %d\n",device_index,index);

		//ATA/ATAPI-4 8.12 (logical pg. 77)
		//	  7 |  6 |   5 |   4 |  3 |  2 |  1 |  0
		//	obs | na | obs | DEV | na | na | na | na
		//	Protocol: PIO data in
		_select_device_as_necessary(device_index);

		kernel->write("Issuing command\n");

		//Write::RegisterOffset::FEATURES_ERRORINFO not applicable
		//Write::RegisterOffset::       NUM_SECTORS not applicable
		//Write::RegisterOffset::           LBA_LOW not applicable
		//Write::RegisterOffset::           LBA_MID not applicable
		//Write::RegisterOffset::          LBA_HIGH not applicable
		_write_uint8(Write::RegisterOffset::DEVICE_HEAD,device_index<<4);
		_current_command                               = 0xEC; _current_command_buffer=data_buffer;
		_write_uint8(Write::RegisterOffset::    COMMAND, 0xEC);

		kernel->write("PIO data in steps 4 and 5\n");

		_proto_pio_data_in(data_buffer+512);

		kernel->write("PIO data in complete!\n");
	#else
		//Inline comments from OSDev http://wiki.osdev.org/ATA_PIO_Mode#IDENTIFY_command

		//"[S]elect a target drive by sending 0xA0 for the master drive, or 0xB0 for the slave, to the \"drive
		//	select\" IO port.  On the Primary bus, this would be port 0x1F6."
		_select_device_as_necessary(device_index);

		//"Then set the Sectorcount, LBAlo, LBAmid, and LBAhi IO ports to 0 (port 0x1F2 to 0x1F5)."
		//	TODO: Write::RegisterOffset::FEATURES_ERRORINFO is not applicable?
		_write_uint8(Write::RegisterOffset::NUM_SECTORS, 0x00);
		_write_uint8(Write::RegisterOffset::    LBA_LOW, 0x00);
		_write_uint8(Write::RegisterOffset::    LBA_MID, 0x00);
		_write_uint8(Write::RegisterOffset::   LBA_HIGH, 0x00);
		//"Then send the IDENTIFY command (0xEC) to the Command IO port (0x1F7)."
		_current_command                               = 0xEC;
		_write_uint8(Write::RegisterOffset::    COMMAND, 0xEC);

		//"Then read the Status port (0x1F7) again."
		for (int i=0;i<5;++i) IO::wait();
		_StatusByte status = _read_uint8(Read::RegisterOffset::STATUS_REGULAR);
		//"If the value read is 0, the drive does not exist."
		if (status.byte==0x00) return false;
		//"For any other value: poll the Status port (0x1F7) until bit 7 (BSY, value = 0x80) clears."
		else {
			do {
				status = _read_uint8(Read::RegisterOffset::STATUS_REGULAR);
			} while (status.bsy);
		}

		//"Because of some ATAPI drives that do not follow spec, at this point you need to check the LBAmid and
		//	LBAhi ports (0x1F4 and 0x1F5) to see if they are non-zero."
		uint8_t  mid = _read_uint8(Read::RegisterOffset:: LBA_MID);
		uint8_t high = _read_uint8(Read::RegisterOffset::LBA_HIGH);
		//"If so, the drive is not ATA, and you should stop polling."
		//	TODO: what?  I'm not polling!
		if (mid || high) return false; //Not supported
		//"Otherwise, continue polling one of the Status ports until bit 3 (DRQ, value = 8) sets, or until bit 0
		//	(ERR, value = 1) sets."
		while (!status.drq && !status.err) {
			status = _read_uint8(Read::RegisterOffset::STATUS_REGULAR);
		}
		//"At that point, if ERR is clear, the data is ready to read from the Data port (0x1F0).  Read 256 16-bit
		//	values, and store them."
		_transfer_block_into(data_buffer);

		return true;
	#endif
}
void Bus::command_readsectors(int device_index, uint8_t* data_buffer, AbsoluteLBA lba,int num_sectors) {
	//ATA/ATAPI-4 8.27 (logical pg. 138)
	//	  7 |   6 |   5 |   4 |    3      2  1   0 |
	//	obs | LBA | obs | DEV | Head number or LBA |
	//	Protocol: PIO data in
	_select_device_as_necessary(device_index);

	assert_term(lba<=268435455ull,"LBA (%u) is larger than (2^28)-1!",static_cast<uint32_t>(lba)); //Only have 28 bits (maximum (2^28)-1 ~= 256 million sectors)
	assert_term(num_sectors<=256,"Number of sectors was larger than 256!"); //Can't be uint8_t, since 256 is valid (we have to pass it as zero though)
	uint8_t num_sectors_arg = num_sectors==256 ? 0 : static_cast<uint8_t>(num_sectors);

	//Write::RegisterOffset::FEATURES_ERRORINFO not applicable
	_write_uint8(Write::RegisterOffset::NUM_SECTORS,              num_sectors_arg);
	_write_uint8(Write::RegisterOffset::    LBA_LOW, (lba&0x00000000000000FF)    );
	_write_uint8(Write::RegisterOffset::    LBA_MID, (lba&0x000000000000FF00)>> 8);
	_write_uint8(Write::RegisterOffset::   LBA_HIGH, (lba&0x0000000000FF0000)>>16);
	_write_uint8(Write::RegisterOffset::DEVICE_HEAD, 0x40 | (device_index<<4) | (lba>>24)); //Note: 0x40 is because using an LBA.  Also note: (lba>>24) is <= 0x0F already.
	_current_command                               = 0x20; _current_command_buffer=data_buffer;
	_write_uint8(Write::RegisterOffset::    COMMAND, 0x20);

	_proto_pio_data_in(data_buffer+512*num_sectors);
}
/*void Bus::command_writesectors(int device_index) {
	//ATA/ATAPI-4 8.48 (logical pg. 207)
	//	  7 |   6 |   5 |   4 |    3      2  1   0 |
	//	obs | LBA | obs | DEV | Head number or LBA |
	//	Protocol: PIO data out
	//0x30
	_select_device_as_necessary(device_index);
	assert_term(false,"Not implemented!");
}*/

//Note: IRQs cannot be interrupted by other IRQs.
void Bus::handle_irq(void) {
	//kernel->write("IRQ begin %d\n", static_cast<int>(_current_command));
	switch (_current_command) {
		case 0xEC:   //IDENTIFY (PIO data in)
		case 0x20: { //READ SECTORS (PIO data in)
			_StatusByte status = _read_uint8(Read::RegisterOffset::STATUS_REGULAR);
			if (status.err) _current_command_buffer=nullptr;
			else {
				assert_term(current!=nullptr,"Current device was null somehow!");
				_transfer_block_into(_current_command_buffer);
				_current_command_buffer += 512;
			}
			break;
		}
		default: assert_term(false,"Interrupt handler for undefined command \"%d\"!",static_cast<int>(_current_command));
	}
	//kernel->write("IRQ end\n");
}

#if 0
void Bus::read_sectors_from_current_drive(AbsoluteLBA lba, uint8_t* data_buffer,int num_sectors) {
	assert_term(current!=nullptr,"Current drive was null somehow!");
	Drive::StatusByte status = current->get_status();
	kernel->write("Begin read; status ");
	status.print();
	kernel->write("\n");

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

	status = current->get_status();
	kernel->write("End read; status ");
	status.print();
	kernel->write("\n");
}
#endif

uint8_t  Bus::  _read_uint8( Read::RegisterOffset offset               ) const {
	assert_term(offset!=Read::RegisterOffset::DATA,"Should use uint16_t method to read data port!");
	return IO::recv<uint8_t>(_io_base+offset);
}
uint16_t Bus:: _read_uint16( Read::RegisterOffset offset               ) const {
	assert_term(offset==Read::RegisterOffset::DATA,"Should use uint8_t method to read registers!");
	return IO::recv<uint16_t>(_io_base+offset);
}
void     Bus:: _write_uint8(Write::RegisterOffset offset,  uint8_t data) const {
	assert_term(offset!=Write::RegisterOffset::DATA,"Should use uint16_t method to write data port!");
	IO::send<uint8_t>(_io_base+offset,data);
}
void     Bus::_write_uint16(Write::RegisterOffset offset, uint16_t data) const {
	assert_term(offset==Write::RegisterOffset::DATA,"Should use uint8_t method to write registers!");
	IO::send<uint8_t>(_io_base+offset,data);
}

void Bus::print(int level) const {
	static char const* bus_names[4] = {"Primary","Secondary","Tertiary","Quaternary"};
	kernel->write_sys(level,"%d: %s bus.  Valid devices:\n",index,bus_names[index]);
	for (int j=0;j<2;++j) {
		Device* device = devices[j];
		if (device==nullptr) {
			kernel->write_sys(level+1,"%d: Invalid drive (or ATAPI)\n",j);
		} else {
			kernel->write_sys(level+1,"%d: Valid drive:\n",j);
			device->print(level+2);
		}
	}
}


}}}
