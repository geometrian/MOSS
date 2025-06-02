#include "stdafx.hpp"

#if   defined MOSS_DEBUG
	#warning "Compiling in debug mode; performance will suffer!"
#elif defined MOSS_RELEASE
#else
	#error "Must define `MOSS_DEBUG` or `MOSS_RELEASE`!"
#endif

#if   defined MOSS_ARCH_IA32
	#pragma message "Compiling MOSS for IA-32 . . ."
#elif defined MOSS_ARCH_AMD64
	#pragma message "Compiling MOSS for AMD64 . . ."
#else
	#error "Must define `MOSS_ARCH_IA32` or `MOSS_ARCH_AMD64`!"
#endif



extern "C" void __moss_uefi_basic_print_ch ( char ch         )
{
	CHAR16 buf[2] = { (CHAR16)ch, '\0' };
	MOSS::g_uefi_sys_table->ConOut->OutputString( MOSS::g_uefi_sys_table->ConOut, buf );
}
extern "C" void __moss_uefi_basic_print_str( char const* str )
{
	CHAR16 buf[512];

	CHAR16* dst = buf;
	while ( *str != '\0' ) *(dst++)=*(str++);
	*dst = '\0';

	MOSS::g_uefi_sys_table->ConOut->OutputString( MOSS::g_uefi_sys_table->ConOut, buf );
}



namespace MOSS
{



EFI_SYSTEM_TABLE* g_uefi_sys_table;



}



extern "C" void __cxa_pure_virtual() noexcept
{
	/*
	If, during runtime, the kernel detects that a call to a pure virtual function couldn't be made,
	it calls the above function.  This function should actually never be called, because without
	corruption/undefined behavior, it is not possible to instantiate a class that doesn't define all
	pure virtual functions.  See also https://wiki.osdev.org/C++
	*/
	/*print_err(); std::printf("Got `__cxa_pure_virtual()` somehow!\n");
	fatalerr();*/
	MOSS::halt();
}
extern "C" void __CxxFrameHandler3() noexcept
{
	/*print_err(); std::printf("Got `CxxFrameHandler3()` (`noexcept` tripped) somehow!\n");
	fatalerr();*/
	MOSS::halt();
}

extern "C" void __std_terminate() noexcept
{
	/*print_err(); std::printf("Got `std::terminate()`\n");
	fatalerr();*/
	MOSS::halt();
}

/*
llvm-project/llvm/docs/ExceptionHandling.rst
__cxa_allocate_exception
__cxa_throw
*/
