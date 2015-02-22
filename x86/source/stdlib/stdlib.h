#pragma once
#include "../includes.h"


void* memset(void* ptr, int value, size_t num);

size_t strlen(const char* str);

char* strncpy(char* destination, const char* source, size_t num);
int strncmp(const char* str1, const char* str2, size_t num);

void delay(int ms);


void* calloc(size_t num, size_t size);

void *operator new  (size_t size);
void *operator new[](size_t size);
void operator delete  (void* p);
void operator delete[](void* p);