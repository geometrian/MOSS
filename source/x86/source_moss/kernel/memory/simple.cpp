#include "simple.h"

#include "../boot/boot_memorymap.h"
#include "../boot/multiboot.h"
#include "../kernel.h"


namespace MOSS { namespace Memory {


uint64_t Block::get_size(void) const {
	return (uint64_t)(header.next) - (uint64_t)(&first_data);
}

void Block::print(void) const {
	kernel->write("{%p<-%p->%p}:%d:[%p,%p)=%d\n", header.prev,this,header.next, header.allocated, &first_data,header.next, get_size());
}


BlockGRUB::BlockGRUB(BlockGRUB const& block) {
	record_size = block.record_size;
	start = block.start;
	size = block.size;
	type = block.type;
}
BlockGRUB::BlockGRUB(Boot::multiboot_memory_map_t* mmap) {
	record_size = mmap->size;
	start = mmap->base_addr;
	size = mmap->length;
	type = mmap->type;
}

void BlockGRUB::print(void) const {
	kernel->write("  size=%d, base_addr=%p %p, len=%p %p, type=%d\n", record_size, start>>32,start&0xffffffff, size>>32,size&0xffffffff, type);
}


MemoryManager::MemoryManager(Boot::multiboot_info_t const* mbi) {
	//GRUB reports some addresses (in particular 0x00000000 to 0x0009FC00) below 1MiB (0x00100000) as free
	//	for use.  However, they aren't for us since we need to retain the ability to set VESA modes.
	//	Therefore, all memory allocation will take place at least at the 1MiB mark.

	//In any case, the linker script loads the kernel to 1MiB, setting up 1MiB of stuff on top of that.
	//	Therefore, all the static, global, etc. data are in [1Mib,2MiB).  Therefore, this heap will look
	//	for a block of data beginning at 1MiB, and then make the heap's start be at 2MiB within.

	//TODO: capture more than one block, if available

	/*terminal->write("mmap_addr=%p, mmap_length=%p\n",mbi->mmap_addr,mbi->mmap_length);

	multiboot_memory_map_t* mmap = (multiboot_memory_map_t*)(mbi->mmap_addr);
	while ((unsigned long)(mmap)<mbi->mmap_addr+mbi->mmap_length) {
		BlockGRUB block(mmap);
		block.print();
		mmap = (multiboot_memory_map_t*)( (unsigned long)(mmap) + mmap->size + sizeof(mmap->size) );
	}*/

	bool found = false;
	Boot::multiboot_memory_map_t* mmap = (Boot::multiboot_memory_map_t*)(mbi->mmap_addr);
	while ((unsigned long)(mmap)<mbi->mmap_addr+mbi->mmap_length) {
		BlockGRUB block(mmap);
		if (block.start==0x00100000) { //MOSS expects to be loaded at 1MiB, so this had better be good.
			if (block.type!=1) break;

			if (block.size <= 0x00200000) break; //Let's say we need 1MiB of RAM to work with for the heap (add another for the kernel stack, global, static, etc.)

			start = 0x00200000;
			end   = start + block.size;

			Block* start2 = (Block*)(start);
			start2->header.allocated = false;
			start2->header.prev = (Block*)(start); //i.e., itself
			start2->header.next = (Block*)(  end);

			found = true;

			break;
		}
		mmap = (Boot::multiboot_memory_map_t*)( (unsigned long)(mmap) + mmap->size + sizeof(mmap->size) );
	}

	assert_term(found,"MemoryManager expected kernel to be loaded at 1MiB and contain a valid memory segment starting there!");
}

void* MemoryManager::malloc(size_t size) {
	Block* block = (Block*)(start);
	LOOP:
		if (block->header.allocated);
		else if (block->get_size() < size);
		else {
			goto END;
		}

		block = block->header.next;
		if ((uint64_t)(block)==end) {
			assert_term(false,"Cannot allocate more space!  Out of space!");
			return nullptr; //No more space!
		}

		goto LOOP;
	END:

	block->header.allocated = true;

	//terminal->write("%p",block);

	//Split this block if can fit another in
	if (block->get_size()-size>sizeof(BlockHeader)) {
		Block* new_block = (Block*)( (uint64_t)(&block->first_data) + size );
		new_block->header.allocated = false;

		new_block->header.next = block->header.next;
		new_block->header.prev = block;

		block->header.next = new_block;

		new_block->header.next->header.prev = new_block;
	}

	return &block->first_data;
}
void MemoryManager::free(void* ptr) {
	Block* block = (Block*)( (uint64_t)(ptr) - sizeof(BlockHeader) );

	block->header.allocated = false;

	//Combine this block as necessary
	Block* new_begin = block;
	Block* new_end = block->header.next;
	Block* prev = block->header.prev;
	Block* next = block->header.next;
	if (!prev->header.allocated) {
		new_begin = prev; //can be a no-op.
	}
	if ((uint64_t)(next)!=end && !next->header.allocated) {
		new_end = next->header.next;
	}
	new_begin->header.next = new_end;
	if ((uint64_t)(new_end)!=end) {
		new_end->header.prev = new_begin;
	}
}

void MemoryManager::print(void) const {
	kernel->write("{\n");
	Block* block = (Block*)(start);
	LOOP:
		kernel->write("  ");
		block->print();
		if ((uint64_t)(block->header.next)!=end) {
			block = block->header.next;
			goto LOOP;
		}
	kernel->write("}\n");
}


}}
