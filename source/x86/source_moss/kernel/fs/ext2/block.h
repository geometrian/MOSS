#pragma once

#include "../../../includes.h"


namespace MOSS { namespace FS {


class Superblock;

class BlockBase {
	private:
		Superblock*const superblock;
	public:
		uint8_t* data;

	protected:
		BlockBase(Superblock* superblock);
	public:
		~BlockBase(void);

		static BlockBase* get_new(Superblock* superblock, uint64_t block_index);
};
class BlockDirect final : public BlockBase {
};
class BlockIndirect1 final : public BlockBase {
};
class BlockIndirect2 final : public BlockBase {
};
class BlockIndirect3 final : public BlockBase {
};


}}