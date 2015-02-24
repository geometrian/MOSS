#pragma once

#include "../../../../includes.h"

#include "common.h"


namespace MOSS { namespace Disk { namespace FileSystem {


/*class INode final {
	uint16_t type_and_permissions;
	uint16_t user_id;
	uint32_t size_lower; //lower 32 bits of size in bytes
	uint32_t time_last_access; //in POSIX time
	uint32_t time_creation; //in POSIX time
	uint32_t time_last_modification; //in POSIX time
	uint32_t time_deletion; //in POSIX time
	uint16_t group_id;
	uint16_t num_hardlinks; //hard links (directory entries) to this inode.  When reaches 0, the data blocks are marked as unallocated.
	uint32_t sectors_used; //disk sectors (not Block-s) in use by this inode, not counting the actual inode structure nor directory entries linking to the inode.
	uint32_t flags; //http://wiki.osdev.org/Ext2#Inode_Flags
	uint32_t os_specific1;
	addrblock pointer[12];
	addrblocki pointeri;
	addrblockii pointerii;
	addrblockiii pointeriii;
	uint32_t generation;
	uint32_t reserved_or_extendedattr;
	uint32_t reserved_or_size_upper_or_acl;
	addrblock fragment;
	uint8_t os_specific2[12];
};*/


}}}
