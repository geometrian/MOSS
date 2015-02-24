#pragma once

#include "../../includes.h"


namespace MOSSC {


void*  memcpy(void*restrict destination, void const*restrict source, size_t num);
void* memmove(void*         destination, void const*         source, size_t num);

char*  strcpy(char*restrict destination, char const*restrict source            );
char* strncpy(char*restrict destination, char const*restrict source, size_t num);


}
