#pragma once

#include "../../includes.h"


namespace MOSSC {


int sprintf(char* str, char const* format, ...);

//TODO: can "buffer" and "format" overlap?
int vsprintf(char* buffer, char const* format, va_list args);


}
