#include "includes.h"

#include "kernel/text_mode_terminal.h"
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

			Kernel::terminal->write("%s:%d: \"%s\"\n", filename,line, buffer);
			LOOP: goto LOOP;
		}

		va_end(args);
	}
	#endif
}