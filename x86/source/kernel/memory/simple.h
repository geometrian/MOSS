#pragma once
#include "../../includes.h"


//TODO: put these inside some namespace
class multiboot_info_t;
class multiboot_memory_map_t;


namespace MOSS { namespace Memory {


class Block;
class BlockHeader { public:
	bool allocated;
	Block* prev;
	Block* next;
};
class Block { public:
	BlockHeader header;
	unsigned int first_data;

	uint64_t get_size(void) const;

	void print(Terminal::TextModeTerminal* terminal) const;
};

class BlockGRUB {
	public:
		unsigned int record_size;
		uint64_t start;
		uint64_t size;
		unsigned int type;

		BlockGRUB(void);
		BlockGRUB(const BlockGRUB& block);
		BlockGRUB(multiboot_memory_map_t* mmap);
		~BlockGRUB(void);

		void print(Terminal::TextModeTerminal* terminal) const;
};

class MemoryManager {
	private:
		uint64_t start; //inclusive
		uint64_t   end; //exclusive

	public:
		MemoryManager(const multiboot_info_t* mbi);
		~MemoryManager(void);

		void* malloc(size_t size);
		void free(void* ptr);

		void print(Terminal::TextModeTerminal* terminal) const;
};


}}