#include "misc.h"


extern "C" void __cxa_pure_virtual(void) {
	//http://wiki.osdev.org/C%2B%2B
	//If, during runtime, the kernel detects that a call to a pure virtual function couldn't be made, it calls the above
	//	function.  This function should actually never be called, because without corruption/undefined behavior, it is not
	//	possible to instantiate a class that doesn't define all pure virtual functions.
	assert_term(false,"__cxa_pure_virtual got called somehow!");
}
