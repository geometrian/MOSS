#pragma once
#include "../../includes.h"


namespace MOSS { namespace Graphics { namespace Font {


//256 characters, each 8x8 pixel-bits, packed into a single 8-byte long long (uint64_t) each
extern uint64_t font[256 * 8*8 / 8];


}}}