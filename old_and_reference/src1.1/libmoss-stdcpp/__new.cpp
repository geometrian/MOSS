/*
Note: this file is named "__new.cpp" instead of "new.cpp" because Make apparently detects that
filename and triggers a compile to it from g++ with no arguments???  Super helpful, guys, thanks.
*/

#include "new"



#ifdef __INTELLISENSE__
	#define MOSS_OPMEM
#else
	//#define MOSS_OPMEM __attribute__((__visibility__("default"))) //__declspec(dllexport)
	#define MOSS_OPMEM __attribute__((weak))
#endif



MOSS_OPMEM [[nodiscard]] void* operator new  ( size_t size                         )
{
	return operator new( size, (std::align_val_t)__STDCPP_DEFAULT_NEW_ALIGNMENT__ );
}
MOSS_OPMEM [[nodiscard]] void* operator new[]( size_t size                         )
{
	return operator new( size );
}
MOSS_OPMEM [[nodiscard]] void* operator new  ( size_t size, std::align_val_t align )
{
	// I think the system expands `size` to a multiple of `align`.  Anyway check it since it's a
	// requirement of `aligned_alloc(⋯)`.
	__moss_assert_impl( (size_t)align>0 && size%(size_t)align==0 );

	void* ptr = std::aligned_alloc( (size_t)align, size );
	__moss_assert( ptr!=nullptr, "Bad alloc!" );
	//if ( ptr == nullptr ) throw std::bad_alloc{};

	return ptr;
}
MOSS_OPMEM [[nodiscard]] void* operator new[]( size_t size, std::align_val_t align )
{
	return operator new( size, align );
}

MOSS_OPMEM void operator delete  ( void* ptr                             ) noexcept
{
	operator delete( ptr, (std::align_val_t)__STDCPP_DEFAULT_NEW_ALIGNMENT__ );
}
MOSS_OPMEM void operator delete[]( void* ptr                             ) noexcept
{
	operator delete( ptr );
}
MOSS_OPMEM void operator delete  ( void* ptr, std::align_val_t /*align*/ ) noexcept
{
	std::free( ptr );
}
MOSS_OPMEM void operator delete[]( void* ptr, std::align_val_t   align   ) noexcept
{
	operator delete( ptr, align );
}



#undef MOSS_OPMEM
