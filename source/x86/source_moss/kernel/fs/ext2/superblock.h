#pragma once

#include "../../../includes.h"


namespace MOSS { namespace FS {


//The superblock isn't really actually a block, so it doesn't inherit from the BlockBase type.
class Superblock {
	public:
		union {
			uint8_t bytes[1024];
			struct {
				//http://wiki.osdev.org/Ext2#Superblock
				uint32_t total_inodes;
				uint32_t total_blocks;
				uint32_t reserved_for_superuser;
				uint32_t unallocated_blocks;
				uint32_t unallocated_inodes;
				uint32_t blocknumber_containing_superblock; //Isn't that already known?
				uint32_t block_size_iv; //intermediate value: shift 1024 to the left by this number
				uint32_t fragment_size_iv; //intermediate value: shift 1024 to the left by this number
				uint32_t blockgroup_num_blocks;
				uint32_t blockgroup_num_fragments;
				uint32_t blockgroup_num_inodes;
				uint32_t last_mount; //in POSIX time
				uint32_t last_write; //in POSIX time
				uint16_t mounts_since_last_check;
				uint16_t mounts_allowable_before_check;
				uint16_t magic; //must be 0xEF53
				uint16_t state; //1=clean, 2=errors
				uint16_t error_action; //1=ignore, 2=remount as read-only, 3=kernel panic
				uint16_t version_minor;
				uint32_t last_check; //in POSIX time
				uint32_t interval_between_forced_checks; //in POSIX time
				uint32_t source_os; //What made the file system: 0=Linux, 1=HURD, 2=MASIX, 3=FreeBSD, 4=BSD4.4 Lite derivative
				uint32_t version_major;
				uint16_t userid_can_use_reserved;
				uint16_t groupid_can_use_reserved;
				uint8_t other_or_unused[1024-(82+2)]; //the above field has offset 82.
			} __attribute__((packed));
		} data;
		int num_blockgroups;
		int block_size;

	public:
		Superblock(void);
		~Superblock(void);

		void print(void) const;
};


}}