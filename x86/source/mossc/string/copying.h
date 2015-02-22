#pragma once

#include <stddef.h>


void* memcpy(void* destination, const void* source, size_t num);
void* memmove(void* destination, const void* source, size_t num);

char* strcpy(char* destination, const char* source);
char* strncpy(char* destination, const char* source, size_t num);