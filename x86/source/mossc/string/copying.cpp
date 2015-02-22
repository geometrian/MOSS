#include "copying.h"


void* memcpy(void* destination, const void* source, size_t num) {
	char* dst = (char*)(destination);
	const char* src = (const char*)(source);

	for (size_t i=0u;i<num;++i) {
		dst[i] = src[i];
	}

	return destination;
}
void* memmove(void* destination, const void* source, size_t num) {
	//Difference is that the pointers can overlap

	if (num!=0u) {
		char* dst = (char*)(destination);
		const char* src = (const char*)(source);

		if (dst>src) {
			//Moving forward in memory (so do the copy backwards)
			size_t i = num - 1;

			LOOP:
				dst[i] = src[i];
				if (i!=0u) {
					i--;
					goto LOOP;
				}
		} else {
			//Moving backward in memory (so do the copy forwards)
			size_t i = 0u;
			do {
				dst[i] = src[i];
			} while (++i<num);
		}
	}

	return destination;
}

char* strcpy(char* destination, const char* source) {
	int i = 0;

	LOOP:
		char c = source[i];
		destination[i] = c;
		if (c!='\0') {
			++i;
			goto LOOP;
		}

	return destination;
}
char* strncpy(char* destination, const char* source, size_t num) {
	size_t i = 0u;

	LOOP1:
		if (i>=num) return destination;

		char c = source[i];
		if (c=='\0') {
			goto LOOP2;
		}
		destination[i] = c;

		++i;
		goto LOOP1;

	LOOP2:
	do {
		destination[i] = '\0';
	} while (++i<num);

	return destination;
}