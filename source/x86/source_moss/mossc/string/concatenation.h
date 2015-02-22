#pragma once

#include <stddef.h>


namespace MOSSC {


char* strcat(char* destination, const char* source);
char* strncat(char* destination, const char* source, size_t num);


}