#pragma once

#include "../../includes.hpp"

#include "../../mosst/linked_list.hpp"


namespace MOSS { namespace Disk {


namespace ATA {
	class Controller;
}


typedef uint64_t AbsoluteLBA;
typedef uint64_t RelativeLBA;

//See http://wiki.osdev.org/Partition_Table#MBR

class HardDiskDrive;
class Partition;

//The maximum number of open sectors.  Do not use a pointer to a "LazySector" that you obtained more than
//	this number of times ago.
#define MOSS_MAX_OPEN_SECTORS 4

class LazySector final {
	friend class HardDiskDrive;
	public:
		HardDiskDrive*const drive;
		Partition* const partition;

		AbsoluteLBA const lba_abs;
		RelativeLBA const lba_rel;

		uint8_t data[512];

	private:
		LazySector(HardDiskDrive* drive, Partition* partition, AbsoluteLBA lba);
	public:
		inline ~LazySector(void) = default; //TODO: should write back data to disk!

		LazySector* get_previous(void) const;
		LazySector*     get_next(void) const;
};

class Partition final {
	public:
		HardDiskDrive*const drive;

		class PartitionTableEntry final { public:
			uint8_t bootable; //0x00 (not) or 0x80 (yes; "active")

			//Don't worry about reading this; use ".relative_sector" instead.
			uint8_t starting_head;
			struct {
				uint8_t  starting_sector   :  6;
				uint16_t starting_cylinder : 10;
			} __attribute__((packed));

			//Hint for the filesystem type (e.g. 0x0B or 0x0C for FAT32).  See:
			//	http://www.win.tue.nl/~aeb/partitions/partition_types-1.html
			uint8_t type_code;

			//Don't worry about reading this; use ".relative_sector" and ".total_sectors" instead.
			uint8_t ending_head;
			struct {
				uint8_t  ending_sector   :  6;
				uint16_t ending_cylinder : 10;
			} __attribute__((packed));

			uint32_t relative_sector; //Relative Sector (to start of partition; also equals the partition's starting LBA value)
			uint32_t   total_sectors; //Total number of sectors in partition
		} __attribute__((packed));
		static_assert(sizeof(PartitionTableEntry)==16,"PartitionTableEntry is the wrong size!");

		PartitionTableEntry*const entry;
		int const index;

	public:
		inline Partition(HardDiskDrive* drive, PartitionTableEntry* entry,int index) : drive(drive), entry(entry),index(index) {}
		inline ~Partition(void) = default;

		LazySector* operator[](RelativeLBA lba) const;

		void  read_sectors(uint8_t*       data_buffer, RelativeLBA lba,int num_sectors) const;
		void write_sectors(uint8_t const* data_buffer, RelativeLBA lba,int num_sectors);

		void print(int level) const;
};

class HardDiskDrive final {
	public:
		ATA::Controller*const controller;

		int const index_bus;
		int const index_device;

		class PartitionTable final { public:
			Partition::PartitionTableEntry entries[4];
		} __attribute__((packed));
		static_assert(sizeof(PartitionTable)==sizeof(Partition::PartitionTableEntry)*4,"PartitionTable is the wrong size!");

		union {
			uint8_t partition_table_data[64];
			PartitionTable partition_table;
		};

		Partition* partitions[4];

	private:
		uint64_t const _HPC; //maximum number of heads per cylinder (reported by disk drive, typically 16 for 28-bit LBA)
		uint64_t const _SPT; //maximum number of sectors per track (reported by disk drive, typically 63 for 28-bit LBA)

		MOSST::LinkedList<LazySector*> _open_sectors;

	public:
		HardDiskDrive(ATA::Controller* controller, int index_bus,int index_device);
		~HardDiskDrive(void);

		LazySector* operator[](AbsoluteLBA lba);

		void  read_sectors(uint8_t*       data_buffer, AbsoluteLBA lba,int num_sectors) const;
		void write_sectors(uint8_t const* data_buffer, AbsoluteLBA lba,int num_sectors);

		//https://en.wikipedia.org/wiki/Logical_block_addressing#CHS_conversion
		AbsoluteLBA chs_to_lba(uint64_t cylinder,uint64_t head,uint64_t sector) const;
		void lba_to_chs(AbsoluteLBA lba, uint64_t*restrict cylinder,uint64_t*restrict head,uint64_t*restrict sector) const;

		void print(int level) const;
};


}}
