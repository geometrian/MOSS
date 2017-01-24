#include "concatenation.hpp"

#include "copying.hpp"
#include "other.hpp"


namespace MOSSC {


char*  strcat(char*restrict destination, char const*restrict source            ) {
	size_t i = strlen(destination);

	strcpy(destination+i,source);

	return destination;
}
char* strncat(char*restrict destination, char const*restrict source, size_t num) {
	char* destination2 = destination + strlen(destination);

	size_t i = 0;
	for (;i<num;++i) {
		char c = source[i];
		if (c=='\0') break;
		destination2[i] = c;
	}

	destination2[i] = '\0';

	return destination;
}


}
