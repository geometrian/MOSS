#pragma once

#include "../../includes.h"


namespace MOSSC {


char*  strcat(char*restrict destination, char const*restrict source            );
//TODO: are these really restricted by the standard?
char* strncat(char*restrict destination, char const*restrict source, size_t num);


}