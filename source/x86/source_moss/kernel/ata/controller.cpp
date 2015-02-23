#include "controller.h"

#include "../kernel.h"

#include "bus.h"
#include "drive.h"


namespace MOSS { namespace ATA {


//Each disk controller chip has probably at least two ATA buses/channels ("primary" and "secondary").
//	Each ATA bus has up to two HDDs attached ("master" and "slave").  This makes for a mess.  As near
//	as I can figure out, the structure looks like this.

//	ATA Controller:
//                             |  IO Port Ranges: |     IRQ:    |
//|----------------------------+------------------+-------------|
//|   Primary ATA Bus/Channel  | [0x01F0,0x01F7], |      14     |
//|                            | [0x03F6,0x03F7]  |             |
//|----------------------------+------------------+-------------|
//|  Secondary ATA Bus/Channel | [0x0170,0x0177], |      15     |
//|   (present on all modern   | [0x0376,0x0377]  |  (probably) |
//|     PCs; often an ATAPI    |                  |             |
//|      optical drive)        |                  |             |
//|----------------------------+------------------+-------------|
//|   Tertiary ATA Bus/Channel | [0x01E8,0x01EF], |      11     |
//|  (probably doesn't exist?) | [0x03EE,0x03EF]  |   (maybe)   |
//|----------------------------+------------------+-------------|
//| Quaternary ATA Bus/Channel | [0x0168,0x016F], |   10 or 11  |
//|  (probably doesn't exist?) | [0x036E,0x036F]  | (who knows) |
//|-------------------------------------------------------------|

//Note: whether an IO operation belongs to the master or the slave drive is a function of which is
//	selected, not the port used.
//Note: the slave's IO overlaps with the floppy disk IO.

//For the IO port ranges, there's great conflict.  It seems likely that the IO port ranges are really
//	a starting offset with characteristic offsets for each register of each drive:

//	Register Offsets:
//		Data Port                           0x0000   Read/write PIO data bytes on this port.
//		Features/Error Information          0x0001   Usually used for ATAPI devices.
//		Number of Sectors                   0x0002   Number of sectors to read/write.  Note: zero is a special value.
//		Sector Number/LBAlo                 0x0003   This is CHS/LBA28/LBA48 specific.
//		Cylinder Low/LBAmid                 0x0004   Partial Disk Sector address.
//		Cylinder High/LBAhi                 0x0005   Partial Disk Sector address.
//		Drive/Head Port                     0x0006   Used to select a drive || head. May support extra address/flag bits.
//		Command Port/Regular Status Port    0x0007   Used to send commands or read the current status.
//		Device Control Register (Writes)/   0x0206   On writes, the device control register changes the behavior of the
//			Alternate Status Register                	ATA bus.  On reads, the alternate status register is returned.
//			(Reads)                                  	This is identical to the status register, except it doesn't hurt
//			                                         	interrupts.

//See also: http://www.pcguide.com/ref/hdd/if/ide/confChannels-c.html
//See also: http://wiki.osdev.org/ATA_PIO_Mode#Registers


Controller::Controller(void) {
	for (int i=0;i<4;++i) {
		buses[i] = new Bus(this, i);
		if (!buses[i]->valid) {
			delete buses[i];
			buses[i] = nullptr;
		}
	}

	kernel->write("  Controller's buses:\n");
	for (int i=0;i<4;++i) {
		Bus* bus = buses[i];
		if (bus==nullptr) {
			kernel->write("    %d: Invalid bus\n",i);
		} else {
			static char const* bus_names[4] = {"Primary","Secondary","Tertiary","Quaternary"};
			kernel->write("    %d: %s bus.  Valid drives:\n",i,bus_names[i]);
			for (int j=0;j<2;++j) {
				Drive* drive = bus->drives[j];
				if (drive==nullptr) {
					kernel->write("      %d: Invalid drive (or ATAPI)\n",j);
				} else {
					kernel->write("      %d: Valid drive:\n",j);
					drive->print(4);
				}
			}
		}
	}
}
Controller::~Controller(void) {
	for (int i=0;i<4;++i) {
		if (buses[i]!=nullptr) delete buses[i];
	}
}

uint8_t const* Controller::read_sector(uint64_t lba) {
	//for (int i=0;i<512;++i) _buffer[i]=66; //debug value

	read_sectors(lba, _buffer,1);

	return _buffer;
}
void Controller::read_sectors(uint64_t lba, uint8_t* data_buffer,int num_sectors) {
	buses[0]->read_sectors_from_current_drive(lba, data_buffer, num_sectors);
}


}}