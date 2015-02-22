#include "includes.h"

#include "kernel/kernel.h"

#include "mossc/cstdio"


namespace MOSS {
	#ifdef MOSS_DEBUG
	void _assert(bool condition, const char* filename,int line, const char* failure_message,...) {
		va_list args;
		va_start(args,failure_message);

		if (!condition) {
			char buffer[256];
			MOSSC::vsprintf(buffer,failure_message,args);

			kernel->write("%s:%d: \"%s\"\n", filename,line, buffer);

			__asm__ __volatile__("cli");
			__asm__ __volatile__("hlt");
			LOOP: goto LOOP;
		}

		va_end(args);
	}
	#endif
}