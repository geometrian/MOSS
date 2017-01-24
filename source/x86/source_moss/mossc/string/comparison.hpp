#pragma once

#include "../../includes.hpp"


namespace MOSSC {


int memcmp(void const* ptr1, void const* ptr2, size_t num);

int  strcmp(char const* str1, char const* str2            );
int strncmp(char const* str1, char const* str2, size_t num);


}
