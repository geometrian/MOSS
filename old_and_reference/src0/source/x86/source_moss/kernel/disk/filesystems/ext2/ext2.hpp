#pragma once

#include "../../../../includes.hpp"

//#include "../../../../mossc/cstring"

#include "../../../kernel.hpp"

#include "../filesystem.hpp"

#include "common.hpp"
#include "superblock.hpp"


namespace MOSS { namespace Disk { namespace FileSystem {


/*using addrblock = uint32_t; //direct pointer
using addrblocki = uint32_t; //indirect pointer
using addrblockii = uint32_t; //doubly indirect pointer
using addrblockiii = uint32_t; //triply indirect pointer
using addrgroup = uint32_t;
using addrinode = uint32_t;

AbsoluteLBA get_lba(Superblock const* superblock, addrblock addr) {
	return addr * superblock->block_size/512;
}
uint32_t get_blockgroup(Superblock const* superblock, addrinode addr) {
	return (addr - 1) / superblock->data.blockgroup_num_inodes; //remember is 1-indexed
}*/

//Does not necessarily fit into one block, so it doesn't subclass BlockBase.
/*class BlockGroupDescriptorTable final {
	public:
		const Superblock*const superblock;

		class BlockGroupDescriptor { public:
			//http://wiki.osdev.org/Ext2#Block_Group_Descriptor
			BlockAddr bitmap_usage_block;
			BlockAddr bitmap_usage_inode;
			BlockAddr inode_table;
			uint16_t num_unallocated_blocks;
			uint16_t num_unallocated_inodes;
			uint16_t num_directories;
			uint8_t unused[32-18];

			inline BlockGroupDescriptor(void) = default;
			inline ~BlockGroupDescriptor(void) = default;
		} __attribute__((packed));
		static_assert(sizeof(BlockGroupDescriptor)==32,"Block group descriptor is the wrong size!");
		BlockGroupDescriptor* groups;
	public:
		BlockGroupDescriptorTable(const Superblock* superblock) : superblock(superblock) {
			//TODO: is this implementation correct for weird sizes?
			addrblock addr; //address of block group descriptor table, which is the block immediately following the superblock
			if (superblock->block_size<=1024) {
				addr = 2048/superblock->block_size;
			} else {
				addr = 1; //must be second block
			}
			AbsoluteLBA lba = get_lba(superblock,addr);

			groups = new BlockGroupDescriptor[superblock->num_blockgroups];
			int i = 0;
			LOOP1:
				uint8_t sector[512];
				kernel->controller_ata->read_sectors(sector, lba,1);
				uint8_t const* sector_pointer = sector;
				size_t j = 0;
				LOOP2:
					kernel->write("Filling group descriptor %d / %d\n",i,superblock->num_blockgroups);
					MOSSC::memcpy(groups+j,sector_pointer,sizeof(BlockGroupDescriptor));
					if (++i==superblock->num_blockgroups) goto END;
					if (++j<512/sizeof(BlockGroupDescriptor)) {
						sector_pointer += sizeof(BlockGroupDescriptor);
						goto LOOP2;
					}
				++lba;
				goto LOOP1;
			END:;
		}
		inline ~BlockGroupDescriptorTable(void) {
			delete [] groups;
		}
};*/

class FileSystemExt2 final : public FileSystemBase {
	private:
		Superblock* _superblock;
		//BlockGroupDescriptorTable* _blockgroup_descriptor_table;

	public:
		explicit FileSystemExt2(Partition* partition) : FileSystemBase(partition) {
			_superblock = new Superblock(this);
			_superblock->print();

			//_blockgroup_descriptor_table = new BlockGroupDescriptorTable(&superblock);
		}
		virtual ~FileSystemExt2(void) {
			//delete _blockgroup_descriptor_table;

			delete _superblock;
		}
};


}}}
