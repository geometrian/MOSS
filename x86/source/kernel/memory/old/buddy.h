#pragma once
#include "../../includes.h"


//Can't be compiled because http://gcc.gnu.org/bugzilla/show_bug.cgi?id=57793 :-(


class multiboot_memory_map_t;

class multiboot_info_t;


//If you change either of these, also change RUN_MACRO_ALL_SIZES
#define MOSS_MEM_BLOCK_SMALLEST 0x00100000u
#define MOSS_MEM_BLOCK_LARGEST  0x80000000u

#define MOSS_MEM_RUN_MACRO_ALL_SIZES(MACRO)\
	MACRO(0x80000000u)\
	MACRO(0x40000000u)\
	MACRO(0x20000000u)\
	MACRO(0x10000000u)\
	MACRO(0x08000000u)\
	MACRO(0x04000000u)\
	MACRO(0x02000000u)\
	MACRO(0x01000000u)\
	MACRO(0x00800000u)\
	MACRO(0x00400000u)\
	MACRO(0x00200000u)\
	MACRO(0x00100000u)
#if MOSS_MEM_BLOCK_SMALLEST != 0x00100000u
	#warning "Adjust RUN_MACRO_ALL_SIZES in manager.h to change SMALLEST_BLOCK!"
#endif
#if MOSS_MEM_BLOCK_LARGEST != 0x80000000u
	#warning "Adjust RUN_MACRO_ALL_SIZES in manager.h to change BLOCK_LARGEST!"
#endif


class BlockInfo { public:
	bool is_subdivided : 1;
	bool     allocated : 1;
	bool       padding : 6;

	void clear(void);
};
template <uint64_t n> class Block { public: //~n MiB
	union {
		struct { Block<(n>>1)> c0,c1; };
		unsigned char data[sizeof(Block<(n>>1)>)*2];
	};
	BlockInfo info;

	void* malloc(size_t size);
	void free(void* ptr);

	void print(TextModeTerminal* terminal) const;
};
template <> class Block<MOSS_MEM_BLOCK_SMALLEST> { public: //n=BLOCK_SMALLEST+1 MiB
	unsigned char data[MOSS_MEM_BLOCK_SMALLEST];
	BlockInfo info;

	void* malloc(size_t/* size*/);
	void free(void*/* ptr*/);

	void print(TextModeTerminal* terminal) const;
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

		void print(TextModeTerminal* terminal) const;
};

class MemoryManager {
	private:
		void* root_block;
		unsigned int root_block_size;

	public:
		MemoryManager(const multiboot_info_t* mbi);
		~MemoryManager(void);

		void* malloc(size_t size);
		void free(void* ptr);

		void print(TextModeTerminal* terminal) const;
};