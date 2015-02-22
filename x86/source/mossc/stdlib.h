#pragma once
#include "../includes.h"


void delay(int ms);


void* calloc(size_t num, size_t size);

void* operator new  (size_t size);
void* operator new[](size_t size);
void operator delete  (void* p);
void operator delete[](void* p);