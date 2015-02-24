#include <crtdbg.h>

#include "../source/mosst/string.h"
#include "../source/mosst/linked_list.h"

#include <stdio.h>


int main(int argc, char* argv[]) {
	#if   PT_BACKEND == PT_BACKEND_WIN32
		_CrtSetDbgFlag(0xFFFFFFFF);
	#endif
	{

		MOSST::String str = "abc123";
		str += "456";
		printf(str.c_str());

		MOSST::LinkedList<int> list;
		list.insert_back(1);
		list.insert_back(2);
		list.insert_back(3);
		list.insert_back(4);
		list.insert_back(5);
		list.remove_back();
		list.remove(2);
	}
	#if   PT_BACKEND == PT_BACKEND_WIN32
		if (_CrtDumpMemoryLeaks()) { int i=0, j=6/i; } //Put a breakpoint here or throw an exception
	#endif

	return 0;
}
