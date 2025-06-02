#pragma once

#include "type_traits"
#include "tuple"



//https://www.scs.stanford.edu/~dm/blog/va-opt.html
#define MOSS_PARENS ()
#define MOSS_EXPAND(...)  MOSS_EXPAND4(MOSS_EXPAND4(MOSS_EXPAND4(MOSS_EXPAND4(__VA_ARGS__))))
#define MOSS_EXPAND4(...) MOSS_EXPAND3(MOSS_EXPAND3(MOSS_EXPAND3(MOSS_EXPAND3(__VA_ARGS__))))
#define MOSS_EXPAND3(...) MOSS_EXPAND2(MOSS_EXPAND2(MOSS_EXPAND2(MOSS_EXPAND2(__VA_ARGS__))))
#define MOSS_EXPAND2(...) MOSS_EXPAND1(MOSS_EXPAND1(MOSS_EXPAND1(MOSS_EXPAND1(__VA_ARGS__))))
#define MOSS_EXPAND1(...) __VA_ARGS__
#define MOSS_FOR_EACH( MACROFN, ARGSEACH, ... )\
	__VA_OPT__( MOSS_EXPAND(_MOSS_FOR_EACH_HELPER( MACROFN, ARGSEACH, __VA_ARGS__ )) )
#define _MOSS_FOR_EACH_HELPER( MACROFN, ARGSEACH, ARG, ... )\
	MACROFN(ARGSEACH,ARG)\
	__VA_OPT__( _MOSS_FOR_EACH_AGAIN MOSS_PARENS (MACROFN,ARGSEACH,__VA_ARGS__) )
#define _MOSS_FOR_EACH_AGAIN() _MOSS_FOR_EACH_HELPER



#define MOSS_INHERIT_1( TYPEPARENT, NAME )\
	using NAME = typename TYPEPARENT::NAME;
#define MOSS_INHERIT_N( TYPEPARENT, ... )\
	MOSS_FOR_EACH( MOSS_INHERIT_1, TYPEPARENT, __VA_ARGS__ )



namespace std
{



template< class Tout,class Tin > [[nodiscard]] constexpr
auto __moss_copy_constness_ptr( Tin* ptr ) noexcept
{
	if constexpr ( is_const_v<Tin> ) return reinterpret_cast<Tout const*>(ptr);
	else                             return reinterpret_cast<Tout      *>(ptr);
}



//https://www.reedbeta.com/blog/python-like-enumerate-in-cpp17/
template< class T > struct _MOSS_EnumeratePair final { size_t ind; T val; };
template<
	class TContainer,
	class TContainerIter = decltype(begin( declval<TContainer>() )),
	class                = decltype(end  ( declval<TContainer>() ))
> [[nodiscard]] constexpr
auto __moss_enumerate( TContainer&& iterable_container, size_t first=0 ) noexcept
{
	struct EnumIter final
	{
		size_t         i;
		TContainerIter iter;

		//Note can't be `<=>` because might be pointer type.
		[[nodiscard]] constexpr auto operator!=( EnumIter const& other ) const noexcept
		{
			return iter != other.iter;
		}
		void operator++() { ++i; ++iter; }
		//[[nodiscard]] constexpr auto operator*() const noexcept { return tie( i,*iter ); }
		[[nodiscard]] constexpr _MOSS_EnumeratePair<decltype(*iter)> operator*() const noexcept
		{
			return { i, *iter };
		}
	};
	struct Wrapper final
	{
		TContainer& container;
		size_t first;
		[[nodiscard]] constexpr EnumIter begin() const noexcept { return { first, ::std::begin(container) }; }
		[[nodiscard]] constexpr EnumIter end  () const noexcept { return { first, ::std::end  (container) }; }
	};
	return Wrapper{ std::forward<TContainer>( iterable_container ), first };
}



template<class T> concept _MOSS_IsContainerDynamic = requires( T& t ) { t.reserve(1); };



}
