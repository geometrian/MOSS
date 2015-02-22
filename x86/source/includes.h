#pragma once
#include <stddef.h>
#include <stdint.h>


#define MOSS_DEBUG

#ifdef MOSS_DEBUG
	#define MOSS_DEBUG_BOCHSBREAK __asm__ __volatile__("xchg  %bx, %bx");
#else
	#define MOSS_DEBUG_BOCHSBREAK
#endif

typedef struct __attribute__((packed)) {
	unsigned short di;
	unsigned short si;

	unsigned short bp;
	unsigned short sp;

	union {
		unsigned short bx;
		struct { unsigned char bl; unsigned char bh; }; //note little-endian
	};
	union {
		unsigned short dx;
		struct { unsigned char dl; unsigned char dh; }; //note little-endian
	};
	union {
		unsigned short cx;
		struct { unsigned char cl; unsigned char ch; }; //note little-endian
	};
	union {
		unsigned short ax;
		struct { unsigned char al; unsigned char ah; }; //note little-endian
	};

	unsigned short gs;
	unsigned short fs;
	unsigned short es;
	unsigned short ds;

	unsigned short eflags;
} regs16_t;

namespace MOSS {
	namespace Terminal {
		class TextModeTerminal;
	}
	namespace Memory {
		class MemoryManager;
	}
	namespace Kernel {
		//TODO: move these to be global variables in their own namespaces
		extern Terminal::TextModeTerminal* terminal;
		extern Memory::MemoryManager* memory;
	}

	#ifdef MOSS_DEBUG
	void moss_assert(bool condition, const char* failure_message);
	#endif
}