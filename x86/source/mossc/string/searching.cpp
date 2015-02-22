#include "searching.h"

#define NULL 0


const void* memchr(const void* ptr, int value, size_t num) {
	const unsigned char* ptr2 = (const unsigned char*)(ptr);
	for (size_t i=0u;i<num;++i) {
		if (ptr2[i]==(unsigned char)(value)) return ptr2+i;
	}
	return NULL;
}
      void* memchr(      void* ptr, int value, size_t num) {
	return (void*)(memchr((const void*)(ptr), value, num));
}

const char* strchr(const char* str, int character) {
	LOOP:
		char c = *str;
		if (c==(char)(character)) return str;
		if (c=='\0') return NULL;
		++str;
		goto LOOP;
}
      char* strchr(      char* str, int character) {
	return (char*)(strchr((const char*)(str),character));
}

size_t strcspn(const char* str1, const char* str2) {
	//Includes NULL-characters in the search
	size_t i = 0u;
	LOOP1:
		char c1 = str1[i];
		size_t j = 0u;
		LOOP2:
			char c2 = str2[j];
			if (c1==c2) return i;
			if (c2!='\0') {
				++j;
				goto LOOP2;
			}
		++i;
		goto LOOP1;
}

const char* strpbrk(const char* str1, const char* str2) {
	//Does not include NULL-characters in the search
	size_t i = 0u;
	LOOP1:
		char c1 = str1[i];
		if (c1=='\0') return NULL;
		size_t j = 0u;
		LOOP2:
			char c2 = str2[j];
			if (c2!='\0') {
				if (c1==c2) return i;
				++j;
				goto LOOP2;
			}
		++i;
		goto LOOP1;
}
      char* strpbrk(      char* str1, const char* str2) {
	return (char*)(strpbrk((const char*)(str1),str2));
}

const char* strrchr(const char* str, int character) {
	LOOP:
		char c = *str;
		if (c==(char)(character)) return str;
		if (c!='\0') {
			++str;
			goto LOOP;
		}
	return NULL;
}
      char* strrchr(      char* str, int character) {
	return (char*)(strrchr((const char*)(str),character));
}

size_t strspn(const char* str1, const char* str2) {
	size_t i = 0u;
	LOOP1:
		char c1 = str1[i];
		if (c1='\0') return i;
		size_t j = 0u;
		LOOP2:
			char c2 = str2[j];
			if (c2!='\0') {
				if (c1==c2) {
					++i;
					goto LOOP1;
				}
				++j;
				goto LOOP2;
			}
		return i;
}

const char* strstr(const char* str1, const char* str2) {
	size_t i = 0u;
	LOOP:
		char c2 = str2[i];
		if (c2=='\0') {
			return str1;
		}
		char c1 = str1[i];
		if (c1=='\0') return NULL;
		if (c1!=c2) {
			++str1;
			i = 0u;
		}
		goto LOOP;
}
      char* strstr(      char* str1, const char* str2) {
	return (char*)(strstr((const char*)(str1),str2));
}