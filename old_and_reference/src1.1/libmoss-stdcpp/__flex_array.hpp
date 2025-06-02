#pragma once

#include "__array_base.hpp"
#include "__util.hpp"

#include "iterator"
#include "limits"



namespace std
{



/*
FlexArray

This type is basically the best of both worlds from a static array and a dynamic array.  A memory
region is allocated statically, and if the contents ever grow beyond that, it seamlessly switches to
dynamic allocation.  This prevents (generally costly) dynamic memory allocation for small data, but
still allows the flexibility of growing large enough.

The main disadvantage of this type is that it's at least the size of the preallocated region.  Also,
because the access is always through a pointer, performance may be very slightly hurt compared to a
static array (you always have to look up and chase a pointer, as opposed to just offsetting into it,
even if the storage is static).  Amortized performance should never be slower than an ordinary
dynamic array though.
*/
template< class T, size_t num_prealloc, class SizeType=size_t, class Alloc=allocator<T> >
class _MOSS_FlexArray /*final*/ : public _MOSS_ArrayBase<
	T, SizeType, _MOSS_FlexArray<T,num_prealloc,SizeType,Alloc>
> {
	static_assert(
		num_prealloc>0 && num_prealloc<=numeric_limits<SizeType>::max(),
		"Invalid minimum preallocation size!"
	);
	static_assert( is_unsigned_v<SizeType> );

	private:
		using _TypeParent = _MOSS_ArrayBase< T, SizeType, _MOSS_FlexArray<T,num_prealloc,SizeType,Alloc> >;
		friend class _MOSS_ArrayBase< T, SizeType, _MOSS_FlexArray<T,num_prealloc,SizeType,Alloc> >;
	public:
		MOSS_INHERIT_N( _TypeParent,
			value_type, size_type, difference_type,
			reference, const_reference,
			iterator, const_iterator
		)
		using       pointer = typename allocator_traits<Alloc>::pointer      ;
		using const_pointer = typename allocator_traits<Alloc>::const_pointer;
		using allocator_type = Alloc;

		enum : size_type { NUM_PREALLOC=num_prealloc };

	private:
		size_type _count, _dyn_capacity;
		struct
		{
			union { T* ptr; byte* dyn; };
			alignas(alignof(T)) byte stat[NUM_PREALLOC*sizeof(T)];
		} _data;
		Alloc _allocator;

		void _init_dyn_capacity_and_prepare_data( size_type count )
		{
			if ( count <= NUM_PREALLOC )
			{
				_dyn_capacity = 0;
				_data.ptr = reinterpret_cast<T*>(_data.stat);
				this->_debug_fill_zeros();
			}
			else
			{
				_dyn_alloc(count);
			}
		}
		template< size_t num_prealloc2, class SizeType2 >
		void _init_from_move( _MOSS_FlexArray<T,num_prealloc2,SizeType2,Alloc>&& other ) noexcept
		{
			if ( other._dyn_capacity == 0 )
			{
				_init_dyn_capacity_and_prepare_data( _count );
				_MOSS_FlexArray::_elems_move( other.data(),data(), _count );
			}
			else
			{
				_dyn_capacity = other._dyn_capacity;
				_data.dyn = other._data.dyn;
				_TypeParent::_debug_fill_zeros( _data.stat, NUM_PREALLOC );
				other._dyn_capacity = 0;
				other._data.ptr = reinterpret_cast<T*>(other._data.stat);
			}
			other._count = 0;
		}

		void _dyn_alloc( size_type count )
		{
			_dyn_capacity = count;
			_data.dyn = reinterpret_cast<byte*>(_allocator.allocate(count));
			this->_debug_fill_zeros();
		}
		void _dyn_dealloc()
		{
			__moss_assert_impl( _dyn_capacity > 0 );
			_allocator.deallocate( reinterpret_cast<T*>(_data.dyn), _dyn_capacity );
		}
		void _dyn_realloc( size_type count )
		{
			__moss_assert_impl(
				count>0 && _count<=count &&
				_dyn_capacity>0 && _dyn_capacity!=count && _count<=_dyn_capacity
			);

			byte* old = _data.dyn;
			_data.dyn = reinterpret_cast<byte*>(_allocator.allocate(count));
			_TypeParent::_debug_fill_zeros( _data.dyn, count );
			_MOSS_FlexArray::_elems_move(
				reinterpret_cast<T*>(old),reinterpret_cast<T*>(_data.dyn), _count
			);
			_allocator.deallocate( reinterpret_cast<T*>(old), _dyn_capacity );

			_dyn_capacity = count;
		}

		static void _elems_move( T*__restrict src,T*__restrict dst, size_type count )
		{
			__moss_assert_impl(__moss_is_all_zeros( dst, count*sizeof(T) ));
			for ( size_type i=0; i<count; ++i )
			{
				new (dst+i) T( std::move(src[i]) );
				src[i].~T();
			}
			_TypeParent::_debug_fill_zeros( src, count );
		}

	public:
		constexpr _MOSS_FlexArray() noexcept(noexcept(Alloc())) : _MOSS_FlexArray(Alloc()) {}
		constexpr explicit _MOSS_FlexArray( Alloc const& allocator ) noexcept :
			_count(0), _allocator(allocator)
		{
			_init_dyn_capacity_and_prepare_data(0);
		}
		constexpr explicit _MOSS_FlexArray( size_type count,              Alloc const& allocator=Alloc() ) :
			_count(count), _allocator(allocator)
		{
			_init_dyn_capacity_and_prepare_data(count);
			for ( size_type i=0; i<count; ++i ) this->_elem_make( i );
		}
		constexpr          _MOSS_FlexArray( size_type count,T const& val, Alloc const& allocator=Alloc() ) :
			_count(count), _allocator(allocator)
		{
			_init_dyn_capacity_and_prepare_data(count);
			for ( size_type i=0; i<count; ++i ) this->_elem_make( i, val );
		}
		template< class InpIter >
		constexpr _MOSS_FlexArray( InpIter first,InpIter last, Alloc const& allocator=Alloc() ) :
			_MOSS_FlexArray(allocator)
		{
			assign( first, last );
		}
		constexpr _MOSS_FlexArray( _MOSS_FlexArray                                  const& other ) :
			_count(other._count), _allocator(other._allocator)
		{
			_init_dyn_capacity_and_prepare_data(_count);
			for ( size_type i=0; i<_count; ++i ) this->_elem_make( i, other[i] );
		}
		template< size_t num_prealloc2, class SizeType2 >
		constexpr _MOSS_FlexArray( _MOSS_FlexArray<T,num_prealloc2,SizeType2,Alloc> const& other ) :
			_count(other._count), _allocator(other._allocator)
		{
			_init_dyn_capacity_and_prepare_data(_count);
			for ( size_type i=0; i<_count; ++i ) this->_elem_make( i, other[i] );
		}
		template< size_t num_prealloc2, class SizeType2 >
		constexpr _MOSS_FlexArray( _MOSS_FlexArray<T,num_prealloc2,SizeType2,Alloc>&& other                  ) noexcept :
			_count(other._count), _allocator(other._allocator)
		{
			_init_from_move( std::forward<_MOSS_FlexArray<T,num_prealloc2,SizeType2,Alloc>>(other) );
		}
		template< size_t num_prealloc2, class SizeType2 >
		constexpr _MOSS_FlexArray( _MOSS_FlexArray<T,num_prealloc2,SizeType2,Alloc>&& other, Alloc allocator )          :
			_count(other._count), _allocator(allocator)
		{
			_init_from_move( std::forward<_MOSS_FlexArray<T,num_prealloc2,SizeType2,Alloc>>(other) );
		}
		constexpr _MOSS_FlexArray( initializer_list<T> ilist, Alloc const& allocator=Alloc() ) :
			_count(ilist.size()), _allocator(allocator)
		{
			_init_dyn_capacity_and_prepare_data(_count);
			for ( auto const& [i,t] : __moss_enumerate(ilist) ) this->_elem_make( i, t );
		}
		constexpr ~_MOSS_FlexArray()
		{
			this->clear();
			if ( _dyn_capacity > 0 ) _dyn_dealloc();
		}

		template< size_t num_prealloc2, class SizeType2, class Alloc2 >
		constexpr _MOSS_FlexArray& operator=( _MOSS_FlexArray<T,num_prealloc2,SizeType2,Alloc2> const& other )
		{
			assign( other.cbegin(), other.cend() );
			return *this;
		}
		template< size_t num_prealloc2, class SizeType2, class Alloc2 >
		constexpr _MOSS_FlexArray& operator=( _MOSS_FlexArray<T,num_prealloc2,SizeType2,Alloc2>&& other )
			noexcept(
				allocator_traits<Alloc>::propagate_on_container_move_assignment::value ||
				allocator_traits<Alloc>::is_always_equal::value
			)
		{
			this->clear();

			_count = other._count;
			if ( other._dyn_capacity == 0 )
			{
				if ( _dyn_capacity>0 && _dyn_capacity<_count )
				{
					_dyn_dealloc();
					_init_dyn_capacity_and_prepare_data(_count);
				}
				_MOSS_FlexArray::_elems_move( other.data(),data(), _count );
			}
			else
			{
				_dyn_capacity = other._dyn_capacity;
				_data.dyn = other._data.dyn;
				_TypeParent::_debug_fill_zeros( _data.stat, NUM_PREALLOC );
				other._dyn_capacity = 0;
				other._data.ptr = other._data.stat;
			}
			other._count = 0;

			return *this;
		}

		template< class InpIter >
		constexpr void assign( InpIter first, InpIter last )
		{
			this->clear();
			if constexpr  (is_same_v<
				typename iterator_traits<InpIter>::iterator_category,
				random_access_iterator_tag
			> ) {
				size_type count = static_cast<size_type>(distance( first, last ));
				reserve( count );
				_count = count;
				for ( size_type i=0; i<count; ) this->_elem_make( i++, *(first++) );
			}
			else
			{
				InpIter iter = first;
				while ( iter != last ) this->insert( this->end(), *(iter++) );
			}
		}

		[[nodiscard]] constexpr allocator_type get_allocator() const noexcept { return _allocator; }

		[[nodiscard]] constexpr T const* data() const noexcept { return _data.ptr; }
		[[nodiscard]] constexpr T      * data()       noexcept { return _data.ptr; }

		[[nodiscard]] constexpr size_type max_size() const noexcept
		{
			return numeric_limits<size_type>::max();
		}
		constexpr void reserve() { reserve( _count + 1 ); } //Note added
		constexpr void reserve( size_type min_capacity )
		{
			if ( min_capacity <= NUM_PREALLOC ) return;

			if ( _dyn_capacity > 0 )
			{
				if ( min_capacity > _dyn_capacity )
				{
					//Note that resizing by the usual power of 2 is actually bad; such ensures that
					//	a steadily growing vector will *always* creep forward in memory, leading to
					//	various problems, like fragmentation.  See also:
					//	https://github.com/facebook/folly/blob/main/folly/docs/FBVector.md
					size_type capacity = _dyn_capacity;
					do { capacity=(3*capacity)/2; } while ( capacity < min_capacity );
					_dyn_realloc( capacity );
				}
			}
			else
			{
				_dyn_alloc( min_capacity );
				_MOSS_FlexArray::_elems_move(
					reinterpret_cast<T*>(_data.stat),reinterpret_cast<T*>(_data.dyn), _count
				);
			}
		}
		[[nodiscard]] constexpr size_type capacity() const noexcept
		{
			return max<size_type>( NUM_PREALLOC, _dyn_capacity );
		}
		constexpr void shrink_to_fit()
		{
			if ( /*_dyn_capacity>0 &&*/ _dyn_capacity>_count ) [[likely]]
			{
				if ( _count <= NUM_PREALLOC )
				{
					_elems_move( _data.dyn,_data.stat, _count );
					_dyn_dealloc();
					_dyn_capacity = 0;
					_data.ptr = reinterpret_cast<T*>(_data.stat);
				}
				else
				{
					_dyn_realloc(_count);
				}
			}
		}

		constexpr void resize( size_type count )
		{
			if ( count == _count ) [[unlikely]] return;

			size_type count_old = _count;
			if ( count > _count )
			{
				reserve(count);
				_count = count;
				for ( size_type i=count_old; i<count; ++i ) this->_elem_make( i );
			}
			else
			{
				for ( size_type i=count; i<count_old; ++i ) this->_elem_cleanup( i );
				_count = count;
			}
		}
		constexpr void resize( size_type count, value_type const& val )
		{
			if ( count == _count ) [[unlikely]] return;

			size_type count_old = _count;
			if ( count > _count )
			{
				reserve(count);
				_count = count;
				for ( size_type i=count_old; i<count; ++i ) this->_elem_make( i, val );
			}
			else
			{
				for ( size_type i=count; i<count_old; ++i ) this->_elem_cleanup( i );
				_count = count;
			}
		}
		constexpr void swap( _MOSS_FlexArray& /*other*/ ) { __moss_notimpl; }
};



template< class T,size_t num_prealloc,class SizeType,class Alloc, class TypeVal > constexpr
SizeType erase   ( _MOSS_FlexArray<T,num_prealloc,SizeType,Alloc>& arr, TypeVal const& value )
{
	return __moss_erase( arr, value );
}
template< class T,size_t num_prealloc,class SizeType,class Alloc, class UnaryPred > constexpr
SizeType erase_if( _MOSS_FlexArray<T,num_prealloc,SizeType,Alloc>& arr, UnaryPred pred )
{
	return __moss_erase_if( arr, pred );
}

template<
	class T,
	size_t num_prealloc1,size_t num_prealloc2,
	class SizeType1,class SizeType2, class Alloc1,class Alloc2
>
constexpr void swap(
	_MOSS_FlexArray<T,num_prealloc1,SizeType1,Alloc1>& lhs,
	_MOSS_FlexArray<T,num_prealloc2,SizeType2,Alloc2>& rhs
) noexcept( noexcept(lhs.swap(rhs)) )
{
	return lhs.swap(rhs);
}



}
