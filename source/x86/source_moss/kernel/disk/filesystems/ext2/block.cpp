#include "block.hpp"

//#include "../../../kernel.hpp"

//#include "../../ata/controller.hpp"

//#include "superblock.hpp"


namespace MOSS { namespace Disk { namespace FileSystem {


/*BlockBase::BlockBase(Superblock* superblock) : superblock(superblock) {
	data = new uint8_t[superblock->block_size];
}
BlockBase::~BlockBase(void) {
	delete [] data;
}
BlockBase* BlockBase::get_new(Superblock* superblock, uint64_t block_index) {
	BlockBase* result = new BlockBase(superblock);
	kernel->controller_ata->read_sectors(result->data, block_index*superblock->block_size/512,superblock->block_size/512);
	return result;
}*/


}}}
