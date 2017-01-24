#include "superblock.hpp"

#include "../../../kernel.hpp"

#include "../../disk.hpp"

#include "ext2.hpp"


namespace MOSS { namespace Disk { namespace FileSystem {


Superblock::Superblock(FileSystemExt2* filesystem) : filesystem(filesystem) {
	//Superblock is at relative LBA 2, 3 (it's 1024 bytes big and starts at byte 1024).
	uint8_t temp[512];
	filesystem->partition->read_sectors(temp, 2ull,1);
	for (int i=0;i<512;++i) data.bytes[i    ]=temp[i];
	filesystem->partition->read_sectors(temp, 3ull,1);
	for (int i=0;i<512;++i) data.bytes[i+512]=temp[i];

	assert_term(data.magic==0xEF53,"Superblock contains wrong magic number (got %X)!",data.magic);

	size_block = 1024 << data.size_block_iv;
	size_fragment = data.size_frag_iv>=0 ? (1024<<data.size_frag_iv) : (1024>>-data.size_frag_iv);

	{
		int num_blockgroups1 = (data.total_blocks+(data.pergroup_blocks-1)) / data.pergroup_blocks; //Note add is for rounding up
		int num_blockgroups2 = (data.total_inodes+(data.pergroup_inodes-1)) / data.pergroup_inodes;
		assert_term(num_blockgroups1==num_blockgroups2,"Block group size check failed!");

		num_blockgroups = num_blockgroups1;
	}
}
Superblock::~Superblock(void) {}

void Superblock::print(void) const {
	kernel->write("Superblock (Ext2 %u.%d):\n",data.revision_major,static_cast<int>(data.revision_minor));
	kernel->write("  Blocks:\n");
	kernel->write("    Free %u + Reserved %u + Allocated %u = Total %u\n",
		data.total_blocks_freereserved - data.total_blocks_reserved,
		data.total_blocks_reserved,
		data.total_blocks - data.total_blocks_freereserved,
		data.total_blocks
	);
	kernel->write("    Block Size:    %u\n",   size_block);
	kernel->write("    Fragment Size: %u\n",size_fragment);
	kernel->write("  INodes:\n");
	kernel->write("    Free %u + Allocated %u = Total %u\n",
		data.total_inodes_free,
		data.total_inodes - data.total_inodes_free,
		data.total_inodes
	);
	kernel->write("  Block Groups:\n");
	kernel->write("    Blocks per group:    %u\n",data.pergroup_blocks);
	kernel->write("    Fragments per group: %u\n",data. pergroup_frags);
	kernel->write("    INodes per group:    %u\n",data.pergroup_inodes);
}


}}}
