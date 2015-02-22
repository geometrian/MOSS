#include "other.h"


void* memset(void* ptr, int value, size_t num) {
	unsigned char* ptr2 = (unsigned char*)(ptr);

	for (size_t i=0u;i<num;++i) {
		*(ptr2+i) = (unsigned char)(value);
	}

	return ptr;
}

size_t strlen(const char* str) {
	size_t ret = 0;
	while (str[ret]!='\0') ++ret;
	return ret;
}