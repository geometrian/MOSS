#include "simple.h"

#include "../boot/bootmemory.h"
#include "../boot/multiboot.h"
#include "../text_mode_terminal.h"
#include "../../stdlib/stdlib.h"


namespace MOSS { namespace Memory {


uint64_t Block::get_size(void) const {
	return (uint64_t)(header.next) - (uint64_t)(&first_data);
}

void Block::print(Terminal::TextModeTerminal* terminal) const {
	/*terminal->write("[");
	terminal->write((void*)(this));
	terminal->write("(+");
	terminal->write((int)(sizeof(BlockHeader)));
	terminal->write("),");
	terminal->write((void*)(header.next));
	terminal->write(")");*/
	terminal->write("{");
	terminal->write((void*)(header.prev));
	terminal->write("<-");
	terminal->write((void*)(this));
	terminal->write("->");
	terminal->write((void*)(header.next));
	terminal->write("}:");
	terminal->write((int)(header.allocated));
	terminal->write(":[");
	terminal->write((void*)(&first_data));
	terminal->write(",");
	terminal->write((void*)(header.next));
	terminal->write(")=");
	terminal->write((int)(get_size()));
	terminal->write("\n");
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

void BlockGRUB::print(Terminal::TextModeTerminal* terminal) const {
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

	//In any case, the linker script loads the kernel to 1MiB, setting up 1MiB of stuff on top of that.
	//Therefore, all the static, global, etc. data are in [1Mib,2MiB).  Therefore, this heap will look
	//for a block of data beginning at 1MiB, and then make the heap's start be at 2MiB within.

	//TODO: capture more than one block, if available

	/*terminal->write("mmap_addr="); terminal->write((void*)(mbi->mmap_addr));
	terminal->write(", mmap_length="); terminal->write((void*)(mbi->mmap_length));
	terminal->write("\n");

	multiboot_memory_map_t* mmap = (multiboot_memory_map_t*)(mbi->mmap_addr);
	while ((unsigned long)(mmap)<mbi->mmap_addr+mbi->mmap_length) {
		BlockGRUB block(mmap);
		block.print(terminal);
		mmap = (multiboot_memory_map_t*)( (unsigned long)(mmap) + mmap->size + sizeof(mmap->size) );
	}*/

	bool found = false;
	multiboot_memory_map_t* mmap = (multiboot_memory_map_t*)(mbi->mmap_addr);
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
		mmap = (multiboot_memory_map_t*)( (unsigned long)(mmap) + mmap->size + sizeof(mmap->size) );
	}

	if (!found) {
		Kernel::terminal->write("MemoryManager expected kernel to be loaded at 1MiB and contain a valid memory segment starting there!");
	}
}
MemoryManager::~MemoryManager(void) {}

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
			return NULL; //No more space!
		}

		goto LOOP;
	END:

	block->header.allocated = true;

	//terminal->write((void*)(block));

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

void MemoryManager::print(Terminal::TextModeTerminal* terminal) const {
	terminal->write("{\n");
	Block* block = (Block*)(start);
	LOOP:
		terminal->write("  ");
		block->print(terminal);
		if ((uint64_t)(block->header.next)!=end) {
			block = block->header.next;
			goto LOOP;
		}
	terminal->write("}\n");
}


}}