#include "other.h"


namespace MOSSC {


void* memset(void* ptr, int value, size_t num) {
	uint8_t* ptr2 = reinterpret_cast<uint8_t*>(ptr);

	for (size_t i=0;i<num;++i) {
		*(ptr2+i) = static_cast<uint8_t>(value);
	}

	return ptr;
}

size_t strlen(char const* str) {
	size_t ret = 0;
	while (str[ret]!='\0') ++ret;
	return ret;
}


}
