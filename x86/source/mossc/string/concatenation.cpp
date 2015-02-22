#include "concatenation.h"

#include "copying.h"


namespace MOSSC {


char* strcat(char* destination, const char* source) {
	size_t i = 0u;
	while (destination[i]!='\0') ++i;
	strcpy(destination+i,source);
	return destination;
}
char* strncat(char* destination, const char* source, size_t num) {
	char* destination2 = destination;

	while (*destination2!='\0') ++destination2;

	size_t i = 0u;
	for (;i<num;++i) {
		char c = source[i];
		if (c=='\0') break;
		destination2[i] = c;
	}

	destination2[i] = '\0';

	return destination;
}


}