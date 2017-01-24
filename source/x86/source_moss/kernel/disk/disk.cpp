#include "disk.hpp"

#include "../kernel.hpp"

#include "ata/controller.hpp"


namespace MOSS { namespace Disk {


LazySector::LazySector(HardDiskDrive* drive, Partition* partition, AbsoluteLBA lba) :
	drive(drive), partition(partition),
	lba_abs(lba), lba_rel(lba-partition->entry->relative_sector)
{
	assert_term(lba>=partition->entry->relative_sector,"Implementation error!");
	drive->read_sectors(data, lba,1);
}

LazySector* LazySector::get_previous(void) const { assert_term(lba_abs>0,"No previous sector exists!"); return (*drive)[lba_abs-1]; }
LazySector* LazySector::    get_next(void) const {                                                      return (*drive)[lba_abs+1]; }


LazySector* Partition::operator[](RelativeLBA lba) const { return (*drive)[entry->relative_sector+lba]; }

void Partition:: read_sectors(uint8_t*       data_buffer, RelativeLBA lba,int num_sectors) const {
	assert_term(
		lba+num_sectors <= entry->total_sectors,
		"Read (offset %u, sectors %d) requested outside of partition (sectors %u)!",
		static_cast<uint32_t>(lba), num_sectors, entry->total_sectors
	);
	drive->read_sectors(data_buffer, entry->relative_sector+lba,num_sectors);
}
void Partition::write_sectors(uint8_t const* data_buffer, RelativeLBA lba,int num_sectors)       {
	assert_term(
		lba+num_sectors <= entry->total_sectors,
		"Write (offset %u, sectors %d) requested outside of partition (sectors %u)!",
		static_cast<uint32_t>(lba), num_sectors, entry->total_sectors
	);
	drive->write_sectors(data_buffer, entry->relative_sector+lba,num_sectors);
}

void Partition::print(int level) const {
	kernel->write_sys(level,"%d: Partition (begin %u, sectors %u)",index,entry->relative_sector,entry->total_sectors);
	if (entry->bootable==0x80) {
		kernel->write(" (bootable)");
	}
	kernel->write("\n");
}


HardDiskDrive::HardDiskDrive(ATA::Controller* controller, int index_bus,int index_device) :
	controller(controller), index_bus(index_bus), index_device(index_device),
	_HPC(16ull), _SPT(63ull) //TODO: better calculation of these!
{
	uint8_t mbr[512];
	read_sectors(mbr, 0,1);

	for (int i=0;i<64;++i) {
		partition_table_data[i] = mbr[0x01BE + i]; //Weird GCC compilation error without space: "0x01BE+i".  TODO: look into it?
	}

	for (int i=0;i<4;++i) {
		//"If a Partition Table entry is unused, then it should be set to all 0."  So, we can just check the
		//	total number of sectors.  If it's zero, then it doesn't exist.  Makes sense.
		if (partition_table.entries[i].total_sectors == 0) {
			partitions[i] = nullptr;
		} else {
			partitions[i] = new Partition(this, partition_table.entries+i,i);
		}
	}
}
HardDiskDrive::~HardDiskDrive(void) {
	while (_open_sectors.size>0) {
		delete _open_sectors.remove_back();
	}

	for (int i=0;i<4;++i) {
		if (partitions[i]!=nullptr) delete partitions[i];
	}
}

LazySector* HardDiskDrive::operator[](AbsoluteLBA lba) {
	for (auto iter=_open_sectors.cbegin(); iter!=_open_sectors.cend(); ++iter) {
		if ((*iter)->lba_abs == lba) return *iter;
	}

	Partition* inside;
	int i = 0;
	LOOP:
		inside = partitions[i];
		if (inside==nullptr || !( lba>=inside->entry->relative_sector && lba<inside->entry->relative_sector+inside->entry->total_sectors )) {
			++i;
			#ifdef MOSS_DEBUG
			if (i==4) {
				kernel->write("Partitions:\n");
				for (int j=0;j<4;++j) {
					if (partitions[j]==nullptr) kernel->write("  (Invalid)\n");
					else kernel->write("  Relative %u, Total %u\n",partitions[j]->entry->relative_sector,partitions[j]->entry->total_sectors);
				}
				kernel->write("Invalid LBA: \"0x ");
				kernel->write("%X ", lba     &0xFF);
				kernel->write("%X ",(lba>> 8)&0xFF);
				kernel->write("%X ",(lba>>16)&0xFF);
				kernel->write("%X ",(lba>>24)&0xFF);
				kernel->write("%X ",(lba>>32)&0xFF);
				kernel->write("%X ",(lba>>40)&0xFF);
				kernel->write("%X ",(lba>>48)&0xFF);
				kernel->write("%X ",(lba>>56)&0xFF);
				assert_term(false,"Invalid LBA: \"!  Not in range of partitions!\n");
			}
			#endif
			goto LOOP;
		}

	LazySector* result = new LazySector(this,inside,lba);
	_open_sectors.insert_back(result);

	if (_open_sectors.size>MOSS_MAX_OPEN_SECTORS) {
		LazySector* closing = _open_sectors.remove_front();
		delete closing;
	}

	return result;
}

void HardDiskDrive:: read_sectors(uint8_t*         data_buffer,   AbsoluteLBA   lba,  int   num_sectors  ) const {
	controller->read_sectors(data_buffer, lba,num_sectors, index_bus,index_device);
}
void HardDiskDrive::write_sectors(uint8_t const* /*data_buffer*/, AbsoluteLBA /*lba*/,int /*num_sectors*/)       {
	assert_term(false,"Not implemented!");
}

AbsoluteLBA HardDiskDrive::chs_to_lba(uint64_t cylinder,uint64_t head,uint64_t sector) const {
	return (cylinder*_HPC + head)*_SPT + (sector-1ull);
}
void HardDiskDrive::lba_to_chs(AbsoluteLBA lba, uint64_t*restrict cylinder,uint64_t*restrict head,uint64_t*restrict sector) const {
	*cylinder =  lba/(_HPC*_SPT);
	*head     = (lba/_SPT)%_HPC;
	*sector   = (lba%_SPT) + 1ull;
}

void HardDiskDrive::print(int level) const {
	kernel->write_sys(level,"Hard disk drive (on ATA bus %d, device %d); partitions:\n",index_bus,index_device);

	for (int i=0;i<4;++i) {
		if (partitions[i]==nullptr) {
			kernel->write_sys(level+1,"%d: Invalid\n",i);
		} else {
			partitions[i]->print(level+1);
		}
	}
}


}}
