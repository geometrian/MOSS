#include "superblock.h"

#include "../../ata/ata.h"
#include "../../kernel.h"


namespace MOSS { namespace FS {


Superblock::Superblock(void) {
	assert_term(sizeof(data)==1024,"Superblock is wrong size %d!",sizeof(data));

	//Superblock is at LBA 2, 3 (it's 1024 bytes big and starts at byte 1024).
	kernel->write("Loading superblock . . .\n");
	const uint8_t* sector2 = kernel->controller_ata->read_sector(2ull); kernel->write("  Got 2!\n");
	for (int i=0;i<512;++i) data.bytes[i    ]=sector2[i];
	const uint8_t* sector3 = kernel->controller_ata->read_sector(3ull); kernel->write("  Got 3!\n");
	for (int i=0;i<512;++i) data.bytes[i+512]=sector3[i];
	kernel->write("Loaded!\n");

	assert_term(data.magic==0xEF53,"Superblock contains wrong magic number (got %p)!",data.magic);

	block_size = 1024 << data.block_size_iv;

	{
		int num_blockgroups1 = data.total_blocks/data.blockgroup_num_blocks;
		if (data.total_blocks%data.blockgroup_num_blocks!=0) ++num_blockgroups1;

		int num_blockgroups2 = data.total_inodes/data.blockgroup_num_inodes;
		if (data.total_inodes%data.blockgroup_num_inodes!=0) ++num_blockgroups2;

		assert_term(num_blockgroups1==num_blockgroups2,"Block group size check failed!");
		num_blockgroups = num_blockgroups1;
	}
}
Superblock::~Superblock(void) {}

/*struct {
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
			} __attribute__((packed));*/

void Superblock::print(void) const {
	kernel->write("Superblock:\n");
	kernel->write("  Blocks:\n");
	kernel->write("    Free/Total: %u/%u\n",data.unallocated_blocks,data.total_blocks);
	kernel->write("    Block Size: %d\n",block_size);
	kernel->write("  INodes:\n");
	kernel->write("    Free/Total: %u/%u\n",data.unallocated_inodes,data.total_inodes);
}


}}