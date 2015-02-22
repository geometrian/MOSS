#include "../source/mosst/string.h"

#include <stdio.h>


int main(int argc, char* argv[]) {
	MOSST::String str = "abc123";
	str += "456";

	printf(str.c_str());

	return 0;
}