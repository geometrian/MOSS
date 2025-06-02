#pragma once

/*#include <cstdarg>
#define __MOSS_WITH_INTS__
#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <cstring>

#include <alloca.h>

#include <bit>*/



#include "../libmoss-stdc/alloca.h"

#include "../libmoss-stdcpp/cctype"
#include "../libmoss-stdcpp/cmath"
#include "../libmoss-stdcpp/cstdarg"
#define __MOSS_WITH_INTS__
#include "../libmoss-stdcpp/cstdint"
#include "../libmoss-stdcpp/cstddef"
#include "../libmoss-stdcpp/cstdio"
#include "../libmoss-stdcpp/cstring"

#include "../libmoss-stdcpp/bit"

#include "../libmoss-stdcpp/array"
#include "../libmoss-stdcpp/list"
#include "../libmoss-stdcpp/string"



using float32 = float ;
using float64 = double;

namespace MOSS
{

template<class...> inline constexpr bool false_v_type = false;
template<auto ...> inline constexpr bool false_v_lit  = false;

}
