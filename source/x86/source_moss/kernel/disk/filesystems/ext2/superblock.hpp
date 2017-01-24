#pragma once

#include "../../../../includes.hpp"

#include "common.hpp"


namespace MOSS { namespace Disk { namespace FileSystem {


class FileSystemExt2;

//Note: the superblock isn't really actually a block, so it doesn't inherit from the BlockBase type.
class Superblock final {
	public:
		FileSystemExt2*const filesystem;

		union {
			uint8_t bytes[1024];
			struct {
				//Descriptions and fields revised from http://www.nongnu.org/ext2-doc/ext2.html#SUPERBLOCK
				//	See also //http://wiki.osdev.org/Ext2#Superblock

				//The total number of inodes (used and free), in the filesystem.  This value must be <=
				//	pergroup_inodes*(number of block groups) and equal to the sum of the inodes defined in each block group.
				uint32_t total_inodes;
				//The total number of blocks in the system (used, free, and reserved).  This value must be <=
				//	pergroup_blocks*(number of block groups).  It must be equal to the sum of the blocks defined in each
				//	block group.
				uint32_t total_blocks;
				//The total number of blocks reserved for the usage of the superuser.  This is most useful if a user fills the
				//	filesystem to capacity; the superuser will have this specified amount of free blocks to fix it.
				uint32_t total_blocks_reserved;

				//The total number of free (including reserved) blocks.  This is the sum of all such blocks in all block groups.
				uint32_t total_blocks_freereserved;
				//The total number of free inodes.  This is the sum of all such inodes in all block groups.
				uint32_t total_inodes_free;

				//The first data block (i.e. the block containing the superblock structure).
				//	Note: this value is always 0 for filesystems with a block size larger than 1KiB, and always 1 for filesystems
				//		with a block size of 1KiB.  The superblock always starts at the 1024th byte of the disk, which
				//		normally happens to be the first byte of the 3rd sector.
				BlockAddr first_data_block;

				//The number of bits to shift left the value 1024 to obtain the block size in bytes.  This value may only be positive.
				//	E.g. "block size = 1024 << size_block_iv;"
				//	Note: common block sizes include 1KiB, 2KiB, 4KiB, and 8Kib.
				//	Note: in Linux, at least up to 2.6.28, the block size must be at least as large as the sector size of the
				//		block device, and cannot be larger than the supported memory page of the architecture.
				uint32_t size_block_iv; //intermediate value
				//The number of bits to shift left the value 1024 to obtain the fragment size in bytes.  Negative values are allowed.
				//	E.g. "fragment size = size_frag_iv>=0 ? (1024<<size_frag_iv) : (1024>>-size_frag_iv);"
				//	Note: as of Linux 2.6.28 no support exists for an Ext2 partition with a fragment size smaller than the
				//		block size, as this feature seems to not be available.
				 int32_t size_frag_iv; //intermediate value

				//The total number of blocks per group.  This, in combination with ".first_data_block", can be used to determine the
				//	block groups' boundaries.
				uint32_t pergroup_blocks;
				//The total number of fragments per group.  Also used to determine the size of the block bitmap of each block group.
				uint32_t pergroup_frags;
				//The total number of inodes per group.  Also used to determine the size of the inode bitmap of each block group.
				//	Note: there cannot be more than (block size in bytes*8) inodes per group, as the inode bitmap must fit within a
				//		single block.  This value must be a perfect multiple of the number of inodes that can fit in a block
				//		((1024<<size_block_iv)/s_inode_size).
				uint32_t pergroup_inodes;

				//Unix time, as defined by POSIX, of the last time the filesystem was mounted.
				uint32_t time_last_mount;
				//Unix time, as defined by POSIX, of the last time a write access was made to the filesystem.
				uint32_t time_last_write;

				//How many times the filesystem was mounted since the last time it was fully verified.
				uint16_t mounts_since_verify;
				//The maximum number of times that the filesystem may be mounted before a full check is performed.
				uint16_t max_mounts_until_verify;

				//Identifies the filesystem as Ext2.  Must be 0xEF53 (EXT2_SUPER_MAGIC in Linux?).
				uint16_t magic;

				//The filesystem state.  When mounting, the read value should be 1 (EXT2_VALID_FS on Linux?).  After being mounted,
				//	the value is changed to 2 (EXT2_ERROR_FS on Linux?).  When unmounting, the 1 (EXT2_VALID_FS) is restored.  In
				//	this way, if the filesystem is not unmounted cleanly (e.g. by a power failure), the value errored value will be
				//	retained.
				uint16_t state;
				//What the driver should do when an error is detected.  Values:
				//	1: Continue as if nothing happened (EXT2_ERRORS_CONTINUE on Linux?)
				//	2:            Remount as read-only (EXT2_ERRORS_RO on Linux?)
				//	3:            Cause a kernel panic (EXT2_ERRORS_PANIC on Linux?)
				uint16_t error_behavior;

				//The minor revision level
				uint16_t revision_minor;

				//Unix time, as defined by POSIX, of the last filesystem check.
				uint32_t time_last_verify;
				//Maximum Unix time interval, as defined by POSIX, allowed between filesystem checks.
				uint32_t max_time_between_verify;

				//Identifier of the OS that created the filesystem.  Values:
				//	0:                   Linux (EXT2_OS_LINUX on Linux?)
				//	1:                GNU HURD (EXT2_OS_HURD on Linux?)
				//	2:                   MASIX (EXT2_OS_MASIX on Linux?)
				//	3:                 FreeBSD (EXT2_OS_FREEBSD on Linux?)
				//	4: BSD 4.4 Lite Derivative (EXT2_OS_LITES on Linux?)
				uint32_t creator_os;

				//The major revision level
				//	0: Revision 0 (EXT2_GOOD_OLD_REV on Linux?)
				//	1: Revision 1 (variable inode sizes, extended attributes, etc.) (EXT2_DYNAMIC_REV on Linux?)
				uint32_t revision_major;

				//The default user ID that can use reserved blocks.
				//	Note: in Linux this defaults to EXT2_DEF_RESUID (value 0).
				uint16_t default_reserved_uid;
				//The default group ID that can use reserved blocks.
				//	Note: in Linux this defaults to EXT2_DEF_RESGID (value 0).
				uint16_t default_reserved_gid;

				//Remainder
				uint8_t _other_or_unused[1024-(82+2)]; //the above field has offset 82.
			} __attribute__((packed));
		} data;
		static_assert(sizeof(data)==1024,"Superblock is wrong size!");

		uint32_t size_block;
		uint32_t size_fragment;
		uint32_t num_blockgroups;

	public:
		explicit Superblock(FileSystemExt2* filesystem);
		inline ~Superblock(void)= default;

		void print(void) const;
};


}}}
