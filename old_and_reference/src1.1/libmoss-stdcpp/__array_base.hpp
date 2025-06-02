#pragma once

#include "cstring"

#include "algorithm"
#include "iterator"
#include "memory"
#include "utility"



namespace std
{



template< class T, class SizeType, class TypeSelf >
class _MOSS_ArrayBase
{
	#define MOSS_SELF __moss_copy_constness_ptr< TypeSelf >(this)
	#define MOSS_COUNT MOSS_SELF->_count
	#define MOSS_DATA  MOSS_SELF->data()
	public:
		using value_type = T;
		using size_type = SizeType;
		using difference_type = ptrdiff_t;
		using reference=value_type&; using const_reference=value_type const&;
		using pointer=value_type*; using const_pointer=value_type const*;
		//using       iterator //LegacyRandomAccessIterator to `value_type`
		//using const_iterator //LegacyRandomAccessIterator to `value_type const`
		using iterator=value_type*; using const_iterator=value_type const*;
		//using       reverse_iterator=reverse_iterator<      iterator>;
		//using const_reverse_iterator=reverse_iterator<const_iterator>;

	protected:
		static void _debug_fill_zeros(void* MOSS_DEBUG_ONLY(ptr),size_type MOSS_DEBUG_ONLY(count)) noexcept
		{
			MOSS_DEBUG_ONLY( memset(ptr,0x00,count*sizeof(T)); )
		}
		void _debug_fill_zeros() noexcept
		{
			_MOSS_ArrayBase::_debug_fill_zeros( MOSS_DATA, MOSS_SELF->capacity() );
		}
		constexpr void _debug_check_mem() const noexcept
		{
			__moss_assert_impl( __moss_is_all_zeros(
				MOSS_DATA + MOSS_COUNT,
				sizeof(T)*( MOSS_SELF->capacity() - MOSS_COUNT )
			));
		}

		void _elem_make( size_type ind_dst )
		{
			__moss_assert_impl(
				ind_dst<MOSS_COUNT && __moss_is_all_zeros( MOSS_DATA+ind_dst, sizeof(T) )
			);
			new (MOSS_DATA+ind_dst) T;
		}
		void _elem_make( size_type ind_dst, T const& src )
		{
			__moss_assert_impl(
				ind_dst<MOSS_COUNT && __moss_is_all_zeros( MOSS_DATA+ind_dst, sizeof(T) )
			);
			new (MOSS_DATA+ind_dst) T(src);
		}
		void _elem_make( size_type ind_dst, T&& src )
		{
			__moss_assert_impl(
				ind_dst<MOSS_COUNT && __moss_is_all_zeros( MOSS_DATA+ind_dst, sizeof(T) )
			);
			new (MOSS_DATA+ind_dst) T( std::forward<T>(src) );
		}
		template< class... Args >
		void _elem_make( size_type ind_dst, Args&&... args )
		{
			__moss_assert_impl(
				ind_dst<MOSS_COUNT && __moss_is_all_zeros( MOSS_DATA+ind_dst, sizeof(T) )
			);
			new (MOSS_DATA+ind_dst) T( std::forward<Args>(args)... );
		}

		void _elem_move_to_empty( size_type isrc, size_type idst )
		{
			_elem_make( idst, std::move((*this)[isrc]) );
			_elem_cleanup( isrc );
		}

		void _elem_cleanup( size_type ind )
		{
			__moss_assert_impl( ind<MOSS_COUNT );
			MOSS_DATA[ind].~T();
			_MOSS_ArrayBase::_debug_fill_zeros( MOSS_DATA+ind, 1 );
		}

	public:
		constexpr TypeSelf& operator=( initializer_list<T> ilist )
		{
			assign(ilist);
			return *MOSS_SELF;
		}

		constexpr void assign( size_type count, T const& val )
		{
			clear();
			MOSS_SELF->resize( count, val );
		}
		constexpr void assign( initializer_list<T> ilist )
		{
			MOSS_SELF->assign( ilist.begin(), ilist.end() );
		}

		/*[[nodiscard]] constexpr const_reference at( size_type ind ) const
		{
			if ( ind<MOSS_COUNT ) [[likely]] return (*this)[ind];
			throw out_of_range( "access array out of bounds" );
		}
		[[nodiscard]] constexpr       reference at( size_type ind )
		{
			if ( ind<MOSS_COUNT ) [[likely]] return (*this)[ind];
			throw out_of_range( "access array out of bounds" );
		}*/
		[[nodiscard]] constexpr const_reference operator[]( size_type ind ) const noexcept
		{
			__moss_assert( ind<MOSS_COUNT, "access array out of bounds" );
			return MOSS_DATA[ ind ];
		}
		[[nodiscard]] constexpr       reference operator[]( size_type ind )       noexcept
		{
			__moss_assert( ind<MOSS_COUNT, "access array out of bounds" );
			return MOSS_DATA[ ind ];
		}
		[[nodiscard]] constexpr const_reference front() const noexcept { return (*this)[           0]; }
		[[nodiscard]] constexpr       reference front()       noexcept { return (*this)[           0]; }
		[[nodiscard]] constexpr const_reference back () const noexcept { return (*this)[MOSS_COUNT-1]; }
		[[nodiscard]] constexpr       reference back ()       noexcept { return (*this)[MOSS_COUNT-1]; }

		//Ideally should leave out `.begin()`/`.end()` in `const` mode; if we must have separate
		//	names for the constant iterators, then aliasing them with the non-const names too is a
		//	misfeature.  However, C++ core features, like foreach, expect it, sooo . . .
		[[nodiscard]] constexpr const_iterator cbegin() const noexcept { return MOSS_DATA; }
		[[nodiscard]] constexpr const_iterator  begin() const noexcept { return MOSS_DATA; }
		[[nodiscard]] constexpr       iterator  begin()       noexcept { return MOSS_DATA; }
		[[nodiscard]] constexpr const_iterator cend  () const noexcept { return MOSS_DATA+MOSS_COUNT; }
		[[nodiscard]] constexpr const_iterator  end  () const noexcept { return MOSS_DATA+MOSS_COUNT; }
		[[nodiscard]] constexpr       iterator  end  ()       noexcept { return MOSS_DATA+MOSS_COUNT; }
		//TODO: reverse iterators

		[[nodiscard]] constexpr bool empty() const noexcept { return MOSS_COUNT==0; }
		[[nodiscard]] constexpr size_type size() const noexcept { return MOSS_COUNT; }

		constexpr void clear() noexcept
		{
			for ( size_type i=0; i<MOSS_COUNT; ++i ) _elem_cleanup(i);
			MOSS_COUNT = 0;
		}

	private:
		iterator _move_left ( const_iterator pos, size_type places )
		{
			difference_type ind = pos - cbegin();
			__moss_assert( ind>=0 && static_cast<size_type>(ind)<MOSS_COUNT, "Invalid iterator!" );

			__moss_assert_impl( places > 0 );
			for ( size_type i=static_cast<size_type>(ind); i<MOSS_COUNT; ++i )
			{
				_elem_cleanup( i );
				if ( i+places < MOSS_COUNT )
				{
					_elem_move_to_empty( i+places, i );
				}
			}
			MOSS_COUNT -= places;

			return begin() + ind;
		}
		iterator _move_right( const_iterator pos, size_type places )
		{
			difference_type ind = pos - cbegin();
			__moss_assert( ind>=0 && static_cast<size_type>(ind)<=MOSS_COUNT, "Invalid iterator!" );

			__moss_assert_impl( places > 0 );

			if constexpr ( _MOSS_IsContainerDynamic<TypeSelf> )
			{
				//Note comes after computing `ind`; this can invalidate `pos`
				//TODO: optimize by inlining `.reserve(...)`.
				MOSS_SELF->reserve( MOSS_COUNT + places );
			}
			else
			{
				__moss_assert( MOSS_COUNT+places<=MOSS_SELF->capacity(), "Too many elements!" );
			}

			difference_type top = static_cast<difference_type>(MOSS_COUNT) - 1;
			MOSS_COUNT += places; //Must come before loop, because of bounds checks
			for ( difference_type i=top; i>=ind; --i )
			{
				_elem_move_to_empty( static_cast<size_type>(i), static_cast<size_type>(i)+places );
			}

			return begin() + ind;
		}
	public:
		constexpr iterator insert( const_iterator pos, T const& val )
		{
			iterator iter = _move_right( pos, 1 );
			_elem_make( static_cast<size_type>(iter-begin()), val );
			return iter;
		}
		constexpr iterator insert( const_iterator pos, T&&      val )
		{
			iterator iter = _move_right( pos, 1 );
			_elem_make( static_cast<size_type>(iter-begin()), std::forward<T>(val) );
			return iter;
		}
		constexpr iterator insert( const_iterator pos, size_type count,T const& val )
		{
			iterator iter = _move_right( pos, count );
			iterator dst = iter;
			for ( size_type i=0; i<count; ++i )
			{
				_elem_make( static_cast<size_type>((dst++)-begin()), val );
			}
			return iter;
		}
		template< class InputIt >
		constexpr iterator insert( const_iterator pos, InputIt first,InputIt last )
		{
			iterator iter = _move_right( pos, distance(first,last) );
			iterator dst = iter;
			for ( InputIt src=first; src!=last; )
			{
				_elem_make( static_cast<size_type>((dst++)-begin()), *(src++) );
			}
			return iter;
		}
		constexpr iterator insert( const_iterator pos, initializer_list<T> ilist )
		{
			return insert( pos, ilist.begin(),ilist.end() );
		}
		template< class... Args >
		constexpr iterator emplace( const_iterator pos, Args&&... args )
		{
			iterator iter = _move_right( pos, 1 );
			__moss_assert_impl( iter >= begin() );
			_elem_make( static_cast<size_type>(iter-begin()), std::forward<Args>(args)... );
			return iter;
		}

		constexpr iterator erase( const_iterator pos ) { return _move_left(pos,1); }
		constexpr iterator erase( const_iterator first, const_iterator last )
		{
			return _move_left( first, distance(first,last) );
		}

		constexpr void push_back( T const& val ) { insert( cend(),                 val  ); }
		constexpr void push_back( T&&      val ) { insert( cend(), std::forward<T>(val) ); }
		template< class... Args >
		constexpr reference emplace_back( Args&&... args )
		{
			return *emplace( cend(), std::forward<Args>(args)... );
		}
		constexpr void pop_back()
		{
			__moss_assert( MOSS_COUNT>0, "Cannot pop an empty array!" );
			_elem_cleanup( MOSS_COUNT - 1 );
			--MOSS_COUNT;
		}

	#undef MOSS_DATA
	#undef MOSS_COUNT
	#undef MOSS_SELF
};



}
