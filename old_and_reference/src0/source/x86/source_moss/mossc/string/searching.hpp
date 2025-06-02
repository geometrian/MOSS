#pragma once

#include "../../includes.hpp"


namespace MOSSC {


void const* memchr(void const* ptr, int value, size_t num);
void*       memchr(void*       ptr, int value, size_t num);

char const* strchr(char const* str, int character);
char*       strchr(char*       str, int character);

size_t strcspn(char const* str1, char const* str2);

char const* strpbrk(char const* str1, char const* str2);
char*       strpbrk(char*       str1, char const* str2);

char const* strrchr(char const* str, int character);
char*       strrchr(char*       str, int character);

size_t strspn(char const* str1, char const* str2);

char const* strstr(char const* str1, char const* str2);
char*       strstr(char*       str1, char const* str2);

//TODO: this
//char* strtok(char* str, char const* delimiters);


}
