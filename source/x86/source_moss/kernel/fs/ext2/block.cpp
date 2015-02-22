#include "block.h"

#include "superblock.h"

#include "../../kernel.h"
#include "../../ata/ata.h"


namespace MOSS { namespace FS {


BlockBase::BlockBase(Superblock* superblock) : superblock(superblock) {
	data = new uint8_t[superblock->block_size];
}
BlockBase::~BlockBase(void) {
	delete [] data;
}
BlockBase* BlockBase::get_new(Superblock* superblock, uint64_t block_index) {
	BlockBase* result = new BlockBase(superblock);
	kernel->controller_ata->read_sectors(block_index*superblock->block_size/512, result->data, superblock->block_size/512);
	return result;
}


}}