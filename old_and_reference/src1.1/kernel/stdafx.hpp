#pragma once

#include <Uefi.h>
#include <Library/UefiLib.h>

#include "../libmoss-util/terminal.hpp"



#define MOSS_MSG   "[ \e[32mINFO\e[0m ] " //MESSAGE, MSG, OK, OKAY
#define MOSS_WARN  "[ \e[33mWARN\e[0m ] "
#define MOSS_ERR   "[ \e[31mFAIL\e[0m ] " //ERROR
#define MOSS_FATAL "[ \e[91mABRT\e[0m ] " //ABRT, QUIT



extern "C" void __moss_uefi_basic_print_ch ( char ch         );
extern "C" void __moss_uefi_basic_print_str( char const* str );



namespace MOSS
{



extern EFI_SYSTEM_TABLE* g_uefi_sys_table;

inline void idle_cpu_until_int() noexcept { asm("hlt"); }
[[noreturn]] inline void halt() noexcept { while (1) idle_cpu_until_int(); }



}
