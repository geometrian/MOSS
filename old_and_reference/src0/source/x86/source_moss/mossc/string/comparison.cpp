#include "comparison.hpp"


namespace MOSSC {


int memcmp(void const* ptr1, void const* ptr2, size_t num) {
	uint8_t const* p1 = reinterpret_cast<uint8_t const*>(ptr1);
	uint8_t const* p2 = reinterpret_cast<uint8_t const*>(ptr2);

	for (size_t i=0;i<num;++i) {
		uint8_t c1 = p1[i];
		uint8_t c2 = p2[i];

		if (c1>c2) return  1;
		if (c1<c2) return -1;
	}

	return 0;
}

int  strcmp(char const* str1, char const* str2            ) {
	uint8_t const* p1 = reinterpret_cast<uint8_t const*>(str1);
	uint8_t const* p2 = reinterpret_cast<uint8_t const*>(str2);

	size_t i = 0;
	LOOP:
		uint8_t c1 = p1[i];
		uint8_t c2 = p2[i];

		if (c1 >  c2) return  1;
		if (c1 <  c2) return -1;
		if (c1=='\0') return  0; //c1==c2, so only test one

		++i;
		goto LOOP;
}
int strncmp(char const* str1, char const* str2, size_t num) {
	uint8_t const* p1 = reinterpret_cast<uint8_t const*>(str1);
	uint8_t const* p2 = reinterpret_cast<uint8_t const*>(str2);

	for (size_t i=0;i<num;++i) {
		uint8_t c1 = p1[i];
		uint8_t c2 = p2[i];

		if (c1 >  c2) return  1;
		if (c1 <  c2) return -1;
		if (c1=='\0') break; //c1==c2, so only test one
	}

	return 0;
}


}
