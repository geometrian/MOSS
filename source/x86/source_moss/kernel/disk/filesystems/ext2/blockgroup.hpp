#pragma once

#include "../../../../includes.hpp"

#include "common.hpp"
#include "superblock.hpp" //TODO: .cpp


namespace MOSS { namespace Disk { namespace FileSystem {


class Superblock;

class BlockGroupDescriptor final {
	public:
		//Descriptions and fields revised from http://www.nongnu.org/ext2-doc/ext2.html#BLOCK-GROUP-DESCRIPTOR-TABLE

		//The first block of the block bitmap for the block group.  The actual block bitmap is located within
		//	its own allocated blocks starting at the block ID specified by this value.
		BlockAddr blockid_block_bitmap;
		//Block id of the first block of the inode bitmap for the block group.
		BlockAddr blockid_inode_bitmap;
		//Block id of the first block of the inode table for the block group.
		BlockAddr blockid_inode_table;

		//The total number of free blocks for the represented group.
		uint16_t total_blocks_free;
		//The total number of free inodes for the represented group.
		uint16_t total_inodes_free;
		//The number of inodes allocated to directories for the represented group.
		uint16_t total_inodes_used_for_dirs;

		//Padding the structure onto a 32-bit boundary.
		uint16_t _pad;

		//12 bytes of reserved space for future revisions.
		uint8_t _other_or_unused[12];
} __attribute__((packed));
static_assert(sizeof(BlockGroupDescriptor)==32,"BlockGroupDescriptor is wrong size!");

/*class BlockGroupDescriptorTable final {
	public:

};*/

class BlockGroup final {
	public:
		//The superblock controlling the filesystem
		Superblock*const root_superblock;

		int const index;

		//Whether this blockgroup contains a superblock and block descriptor table
		bool has_sb_and_bdt;

	public:
		BlockGroup(Superblock* superblock, int index) : root_superblock(superblock), index(index) {
			//Note: "Shadow copies of the block group descriptor table are also stored with every copy of the superblock."
			if (superblock->data.revision_major == 0) {
				has_sb_and_bdt = true;
			} else {
				assert_term(superblock->data.revision_major==1,"Unexpected Ext2 revision \"%u\"!",superblock->data.revision_major);
				if (index==0 || index==1 || index%3==0 || index%5==0 || index%7==0) {
					has_sb_and_bdt =  true;
				} else {
					has_sb_and_bdt = false;
				}
			}


		}
		inline ~BlockGroup(void) {}
};


}}}
