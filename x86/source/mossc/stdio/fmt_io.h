#pragma once

#include <stdarg.h>


namespace MOSSC {


int sprintf(char* str, const char* format, ...);

int vsprintf(char* buffer, const char* format, va_list args);


}