#pragma once
#include "../fs.h"

#include "../../../mossc/cstring"

#include "../../ata/ata.h"
#include "../../kernel.h"

#include "superblock.h"


namespace MOSS { namespace FS {


//http://wiki.osdev.org/Ext2

typedef uint32_t addrblock; //direct pointer
typedef uint32_t addrblocki; //indirect pointer
typedef uint32_t addrblockii; //doubly indirect pointer
typedef uint32_t addrblockiii; //triply indirect pointer
typedef uint32_t addrgroup;
typedef uint32_t addrinode;

class Superblock;

uint32_t get_lba(const Superblock* superblock, addrblock addr) {
	return addr * superblock->block_size/512;
}
uint32_t get_blockgroup(const Superblock* superblock, addrinode addr) {
	return (addr - 1) / superblock->data.blockgroup_num_inodes; //remember is 1-indexed
}

//Does not necessarily fit into one block, so it doesn't subclass BlockBase.
class BlockGroupDescriptorTable {
	public:
		const Superblock*const superblock;

		class BlockGroupDescriptor { public:
			//http://wiki.osdev.org/Ext2#Block_Group_Descriptor
			addrblock bitmap_usage_block;
			addrblock bitmap_usage_inode;
			addrblock inode_table;
			uint16_t num_unallocated_blocks;
			uint16_t num_unallocated_inodes;
			uint16_t num_directories;
			uint8_t unused[32-18];

			BlockGroupDescriptor(void) {}
			~BlockGroupDescriptor(void) {}
		} __attribute__((packed));
		BlockGroupDescriptor* groups;
	public:
		BlockGroupDescriptorTable(const Superblock* superblock) : superblock(superblock) {
			ASSERT(sizeof(BlockGroupDescriptor)==32,"Block group descriptor is the wrong size!");

			//TODO: is this implementation correct for weird sizes?
			addrblock addr; //address of block group descriptor table, which is the block immediately following the superblock
			if (superblock->block_size<=1024) {
				addr = 2048/superblock->block_size;
			} else {
				addr = 1; //must be second block
			}
			uint64_t lba = get_lba(superblock,addr);

			groups = new BlockGroupDescriptor[superblock->num_blockgroups];
			int i = 0;
			LOOP1:
				const uint8_t* sector = kernel->controller_ata->read_sector(lba);
				//The 32 in this loop is sizeof(BlockGroupDescriptor), replaced with its value for clarity.  Note
				//the check for this equivalence above.
				int j = 0;
				LOOP2:
					kernel->write("Filling group descriptor %d / %d\n",i,superblock->num_blockgroups);
					MOSSC::memcpy(groups+j,sector,32);
					if (++i==superblock->num_blockgroups) goto END;
					if (++j<512/32) {
						sector += 32;
						goto LOOP2;
					}
				++lba;
				goto LOOP1;
			END:;
		}
		~BlockGroupDescriptorTable(void) {
			delete [] groups;
		}
};

//http://www.nongnu.org/ext2-doc/ext2.html#DISK-ORGANISATION
class InterfaceFileSystemEXT2 : public InterfaceFileSystemBase {
	private:
		Superblock superblock;
		BlockGroupDescriptorTable* blockgroup_descriptor_table;
	public:
		InterfaceFileSystemEXT2(void) : superblock() {
			kernel->write("Setting up ext2 filesystem!\n");
			superblock.print();
			//blockgroup_descriptor_table = new BlockGroupDescriptorTable(&superblock);
		}
		~InterfaceFileSystemEXT2(void) {
			//delete blockgroup_descriptor_table;
		}
};


}}