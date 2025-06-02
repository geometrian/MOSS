#include "searching.hpp"


namespace MOSSC {


void const* memchr(void const* ptr, int value, size_t num) {
	uint8_t const* ptr2 = reinterpret_cast<uint8_t const*>(ptr);
	for (size_t i=0;i<num;++i) {
		if (ptr2[i]==static_cast<uint8_t>(value)) return ptr2+i;
	}
	return nullptr;
}
void*       memchr(void*       ptr, int value, size_t num) {
	return const_cast<void*>(memchr(const_cast<void const*>(ptr), value, num));
}

char const* strchr(char const* str, int character) {
	LOOP:
		char c = *str;
		if (c==static_cast<char>(character)) return str;
		if (c!='\0') {
			++str;
			goto LOOP;
		}
	return nullptr;
}
char*       strchr(char*       str, int character) {
	return const_cast<char*>(strchr(const_cast<char const*>(str), character));
}

size_t strcspn(char const* str1, char const* str2) {
	//Includes null-characters in the search
	size_t i = 0;
	LOOP1:
		char c1 = str1[i];
		size_t j = 0;
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

char const* strpbrk(char const* str1, char const* str2) {
	//Does not include null-characters in the search
	size_t i = 0;
	LOOP1:
		char c1 = str1[i];
		if (c1=='\0') return nullptr;
		size_t j = 0;
		LOOP2:
			char c2 = str2[j];
			if (c2!='\0') {
				if (c1==c2) return str1+i;
				++j;
				goto LOOP2;
			}
		++i;
		goto LOOP1;
}
char*       strpbrk(char*       str1, char const* str2) {
	return const_cast<char*>(strpbrk(const_cast<char const*>(str1),str2));
}

char const* strrchr(char const* str, int character) {
	char const* last = nullptr;
	LOOP:
		char c = *str;
		if (c==static_cast<char>(character)) last=str;
		if (c!='\0') {
			++str;
			goto LOOP;
		}
	return last;
}
char*       strrchr(char*       str, int character) {
	return const_cast<char*>(strrchr(const_cast<char const*>(str), character));
}

size_t strspn(char const* str1, char const* str2) {
	size_t i = 0;
	LOOP1:
		char c1 = str1[i];
		if (c1=='\0') return i;

		size_t j = 0;
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

char const* strstr(char const* str1, char const* str2) {
	size_t i = 0;
	LOOP:
		char c2 = str2[i];
		if (c2=='\0') return str1;

		char c1 = str1[i];
		if (c1=='\0') return nullptr;

		if (c1!=c2) {
			++str1;
			i = 0;
		} else {
			++i;
		}

		goto LOOP;
}
char*       strstr(char*       str1, char const* str2) {
	return const_cast<char*>(strstr(const_cast<char const*>(str1), str2));
}


}
