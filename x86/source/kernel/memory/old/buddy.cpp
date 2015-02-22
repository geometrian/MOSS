#include "manager.h"

#include "../boot/bootmemory.h"
#include "../boot/multiboot.h"
#include "../text_mode_terminal.h"


void BlockInfo::clear(void) {
	is_subdivided = false;
	allocated = false;
}


template <uint64_t n> void* Block<n>::malloc(size_t size) {
	if (info.allocated) return NULL;

	if (size>sizeof(Block<(n>>1)>)) {
		if (info.is_subdivided) return NULL;
		else {
			info.allocated = true;
			return data;
		}
	} else {
		if (!info.is_subdivided) {
			info.is_subdivided = true;
			c0.info.clear();
			c1.info.clear();
		}

		void* result;
		result = c0.malloc(size);
		if (result == NULL) {
			result = c1.malloc(size);
		}

		info.allocated = c0.info.allocated && c1.info.allocated;

		return result;
	}
}
template <uint64_t n> void Block<n>::free(void* ptr) {
	if (info.is_subdivided) { //was allocated in a lower level
		if (ptr>=&c1) {
			c1.free(ptr);
		} else {
			c0.free(ptr);
		}
		if (!c0.info.allocated&&!c1.info.allocated) {
			info.clear();
		}
	} else { //we allocated ptr
		info.allocated = false;
	}
}

template <uint64_t n> void Block<n>::print(TextModeTerminal* terminal) const {
	unsigned int a = n;
	while (a<MOSS_MEM_BLOCK_LARGEST) {
		a <<= 1;
		terminal->write("~");
	}
	terminal->write("Allocated: ");
	terminal->write(info.allocated);
	terminal->write(", Subdivided: ");
	terminal->write(info.is_subdivided);
	if (n==MOSS_MEM_BLOCK_LARGEST) {
		terminal->write(", (top level)");
	}
	terminal->write("\n");
	if (info.is_subdivided) {
		c0.print(terminal);
		c1.print(terminal);
	}
}


#define INSTANTIATE(SIZE)\
	template class Block<SIZE>;
MOSS_MEM_RUN_MACRO_ALL_SIZES(INSTANTIATE)
#undef INSTANTIATE


void* Block<MOSS_MEM_BLOCK_SMALLEST>::malloc(size_t/* size*/) { //we know that size is smaller than 0x00100000
	if (info.allocated) return NULL;

	info.allocated = true;

	return data;
}
void Block<MOSS_MEM_BLOCK_SMALLEST>::free(void*/* ptr*/) { //we know we allocated this pointer
	info.allocated = false;
}

void Block<MOSS_MEM_BLOCK_SMALLEST>::print(TextModeTerminal* terminal) const {
	terminal->write("Allocated: ");
	terminal->write(info.allocated);
	terminal->write(", (lowest level)\n");
}


BlockGRUB::BlockGRUB(void) {}
BlockGRUB::BlockGRUB(const BlockGRUB& block) {
	record_size = block.record_size;
	start = block.start;
	size = block.size;
	type = block.type;
}
BlockGRUB::BlockGRUB(multiboot_memory_map_t* mmap) {
	record_size = mmap->size;
	start = mmap->base_addr;
	size = mmap->length;
	type = mmap->type;
}
BlockGRUB::~BlockGRUB(void) {
}

void BlockGRUB::print(TextModeTerminal* terminal) const {
	terminal->write("  size="); terminal->write((int)(record_size));
	terminal->write(", base_addr="); terminal->write((void*)(start >> 32)); terminal->write(" "); terminal->write((void*)(start & 0xffffffff));
	terminal->write(", len="); terminal->write((void*)(size >> 32)); terminal->write(" "); terminal->write((void*)(size & 0xffffffff));
	terminal->write(", type="); terminal->write((int)(type));
	terminal->write("\n");
}


MemoryManager::MemoryManager(const multiboot_info_t* mbi) {
	//GRUB reports some addresses (in particular 0x00000000 to 0x0009FC00) below 1MiB as free for use.
	//However, they aren't for us since we need to retain the ability to set VESA modes.  Therefore,
	//all memory allocation will take place at least at the 1MiB mark.

	//TODO: capture more than one block, if available
	//TODO: fill in block more completely if possible

	/*terminal->write("mmap_addr="); terminal->write((void*)(mbi->mmap_addr));
	terminal->write(", mmap_length="); terminal->write((void*)(mbi->mmap_length));
	terminal->write("\n");*/

	/*#define OUTPUT_SIZEOF(N)\
		terminal->write((void*)(N));\
		terminal->write(": ");\
		terminal->write((int)(sizeof(Block<N>)));\
		terminal->write(" (");\
		terminal->write((void*)(sizeof(Block<N>)));\
		terminal->write(")\n");

	OUTPUT_SIZEOF(0x00100000u)
	OUTPUT_SIZEOF(0x00200000u)
	OUTPUT_SIZEOF(0x00400000u)
	OUTPUT_SIZEOF(0x00800000u)
	OUTPUT_SIZEOF(0x01000000u)
	OUTPUT_SIZEOF(0x02000000u)
	OUTPUT_SIZEOF(0x04000000u)
	OUTPUT_SIZEOF(0x08000000u)
	OUTPUT_SIZEOF(0x10000000u)
	OUTPUT_SIZEOF(0x20000000u)
	OUTPUT_SIZEOF(0x40000000u)
	OUTPUT_SIZEOF(0x80000000u)*/

	/*int num_blocks = 0;
	BlockGRUB first_block;

	multiboot_memory_map_t* mmap = (multiboot_memory_map_t*)(mbi->mmap_addr);
	while ((unsigned long)(mmap)<mbi->mmap_addr+mbi->mmap_length) {
		BlockGRUB block(mmap);
		if      (block.start<0x00100000); //Do not use anything below 1MiB
		else if (block.type!=1); //Only valid blocks
		else {
			block.print(terminal);
			if (num_blocks++==0) {
				first_block = block;
			}
		}
		mmap = (multiboot_memory_map_t*)( (unsigned long)(mmap) + mmap->size + sizeof(mmap->size) );
	}*/

	root_block = NULL;
	root_block_size = 0;

	multiboot_memory_map_t* mmap = (multiboot_memory_map_t*)(mbi->mmap_addr);
	while ((unsigned long)(mmap)<mbi->mmap_addr+mbi->mmap_length) {
		BlockGRUB block(mmap);
		if      (block.start<0x00100000); //Do not use anything below 1MiB
		else if (block.type!=1); //Only valid blocks
		else {
			#define CHECK(SIZE)\
				if (block.size>SIZE) {\
					Block<SIZE>* block2 = (Block<SIZE>*)(block.start);\
					block2->info.clear();\
					root_block = block2;\
					root_block_size = SIZE;\
					break;\
				}
			MOSS_MEM_RUN_MACRO_ALL_SIZES(CHECK)
			#undef CHECK
		}
		mmap = (multiboot_memory_map_t*)( (unsigned long)(mmap) + mmap->size + sizeof(mmap->size) );
	}
}
MemoryManager::~MemoryManager(void) {
}

void* MemoryManager::malloc(size_t size) {
	switch (root_block_size) {
		#define MALLOC(SIZE)\
			case SIZE: return ((Block<SIZE>*)(root_block))->malloc(size);
		MOSS_MEM_RUN_MACRO_ALL_SIZES(MALLOC)
		#undef MALLOC
	}
	return NULL;
}
void MemoryManager::free(void* ptr) {
	switch (root_block_size) {
		#define FREE(SIZE)\
			case SIZE: return ((Block<SIZE>*)(root_block))->free(ptr);
		MOSS_MEM_RUN_MACRO_ALL_SIZES(FREE)
		#undef FREE
	}
}

void MemoryManager::print(TextModeTerminal* terminal) const {
	switch (root_block_size) {
		#define PRINT(SIZE)\
			case SIZE: return ((Block<SIZE>*)(root_block))->print(terminal);
		MOSS_MEM_RUN_MACRO_ALL_SIZES(PRINT)
		#undef PRINT
	}
}