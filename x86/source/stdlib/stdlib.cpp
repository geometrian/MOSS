#include "stdlib.h"

#include "../includes.h"

#include "../kernel/memory/simple.h"
#include "../kernel/kernel.h"


void* memset(void* ptr, int value, size_t num) {
	unsigned char* ptr2 = (unsigned char*)(ptr);

	for (size_t i=0;i<num;++i) {
		*(ptr2+i) = (unsigned char)(value);
	}

	return ptr;
}

size_t strlen(const char* str) {
	size_t ret = 0;
	while (str[ret]!='\0') ++ret;
	return ret;
}

char* strncpy(char* destination, const char* source, size_t num) {
	size_t i = 0;
	for (;i<num;++i) {
		char c = source[i];
		if (c=='\0') break;
		destination[i] = c;
	}
	for (;i<num;++i) {
		destination[i] = '\0';
	}
	return destination;
}
int strncmp(const char* str1, const char* str2, size_t num) {
	for (size_t i=0;i<num;++i) {
		char c0 = str1[i];
		char c1 = str2[i];
		if (c0>c1) return  1;
		if (c1<c0) return -1;
		if (c0=='\0') { //c0==c1, so only test one
			break;
		}
	}
	return 0;
}

void delay(int ms) { //approximately correct on VirtualBox
	ms /= 2;
	volatile int counter = 0;
	for (int i=0;i<1000;++i) {
		for (int j=0;j<1000;++j) {
			for (int k=0;k<ms;++k) {
				++counter;
			}
		}
	}
}


void* calloc(size_t num, size_t size) {
	#ifdef MOSS_DEBUG
	ASSERT(MOSS::Kernel::memory!=NULL,"Cannot allocate memory; memory manager does not exist!");
	#endif
	void* result = MOSS::Kernel::memory->malloc(num*size);
	memset(result,0,size);
	return result;
}

void* operator new  (size_t size) {
	#ifdef MOSS_DEBUG
	ASSERT(MOSS::Kernel::memory!=NULL,"Cannot allocate memory; memory manager does not exist!");
	#endif
	return MOSS::Kernel::memory->malloc(size);
}
void* operator new[](size_t size) {
	#ifdef MOSS_DEBUG
	ASSERT(MOSS::Kernel::memory!=NULL,"Cannot allocate memory; memory manager does not exist!");
	#endif
	return MOSS::Kernel::memory->malloc(size);
}
void operator delete  (void* p) {
	#ifdef MOSS_DEBUG
	ASSERT(MOSS::Kernel::memory!=NULL,"Cannot delete memory; memory manager does not exist!");
	#endif
	MOSS::Kernel::memory->free(p);
}
void operator delete[](void* p) {
	#ifdef MOSS_DEBUG
	ASSERT(MOSS::Kernel::memory!=NULL,"Cannot delete memory; memory manager does not exist!");
	#endif
	MOSS::Kernel::memory->free(p);
}