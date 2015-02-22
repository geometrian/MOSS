#pragma once
#include <stddef.h>
#include <stdint.h>


#define MOSS_DEBUG

#ifdef MOSS_DEBUG
	#define MOSS_DEBUG_BOCHSBREAK __asm__ __volatile__("xchg  %bx, %bx");
#else
	#define MOSS_DEBUG_BOCHSBREAK
#endif

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
		void _assert(bool condition, const char* failure_message);
		#define assert(CONDITION,FAILURE_MESSAGE) _assert(CONDITION,FAILURE_MESSAGE);
	#else
		#define assert(CONDITION,FAILURE_MESSAGE)
	#endif
}