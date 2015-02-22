#include "comparison.h"


namespace MOSSC {


int memcmp(const void* ptr1, const void* ptr2, size_t num) {
	const unsigned char* p1 = (const unsigned char*)(ptr1);
	const unsigned char* p2 = (const unsigned char*)(ptr2);
	for (size_t i=0u;i<num;++i) {
		unsigned char c1 = p1[i];
		unsigned char c2 = p2[i];
		if (c1>c2) return  1;
		if (c1<c2) return -1;
	}
	return 0;
}

int strcmp(const char* str1, const char* str2) {
	const unsigned char* p1 = (const unsigned char*)(str1);
	const unsigned char* p2 = (const unsigned char*)(str2);
	size_t i = 0u;
	LOOP:
		unsigned char c1 = p1[i];
		unsigned char c2 = p2[i];
		if (c1 >  c2) return  1;
		if (c1 <  c2) return -1;
		if (c1=='\0') return  0;
		goto LOOP;
}
int strncmp(const char* str1, const char* str2, size_t num) {
	const unsigned char* p1 = (const unsigned char*)(str1);
	const unsigned char* p2 = (const unsigned char*)(str2);
	for (size_t i=0u;i<num;++i) {
		unsigned char c1 = p1[i];
		unsigned char c2 = p2[i];
		if (c1 >  c2) return  1;
		if (c1 <  c2) return -1;
		if (c1=='\0') break; //c1==c2, so only test one
	}
	return '\0';
}


}