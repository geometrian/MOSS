#pragma once

#include <stddef.h>


namespace MOSSC {


void* memset(void* ptr, int value, size_t num);

//TODO: this
char* strerror(int errnum);

size_t strlen(const char* str);


}