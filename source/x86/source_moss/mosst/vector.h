#pragma once

#include "../includes.h"

#include "../mossc/cstring"


namespace MOSST {


//Note not final; useful for e.g. String to subclass.
template <typename type> class Vector {
	protected:
		uint8_t* _data;
		int _capacity;
	public:
		//User can read, but should not change.
		int size;

	public:
		Vector(void) {
			_data = nullptr;
			_capacity = 0;
			size = 0;
		}
		Vector(Vector const& other) : Vector() {
			reserve(other.size);
			for (int i=0;i<other.size;++i) reinterpret_cast<type*>(_data)[i]=reinterpret_cast<type*>(other._data)[i];
		}
		inline virtual ~Vector(void) {
			if (_data!=nullptr) delete [] _data;
		}

		inline type* get_backing(void) { return reinterpret_cast<type*>(_data); }

		Vector& operator=(Vector const& other) {
			size = 0;
			reserve(other.size);
			for (int i=0;i<other.size;++i) reinterpret_cast<type*>(_data)[i]=reinterpret_cast<type*>(other._data)[i];
			size = other.size;
			return *this;
		}

		//Reserves space for at least the given number of elements.  Returns whether a reallocation happened.
		bool reserve(int num_elements) {
			if (_capacity>=num_elements) return false;

			uint8_t* data2 = new uint8_t[num_elements*sizeof(type)];
			if (_data!=nullptr) {
				//if (size>0) MOSSC::memcpy(data2,_data, size*sizeof(type));
				for (int i=0;i<size;++i) reinterpret_cast<type*>(data2)[i]=reinterpret_cast<type*>(_data)[i];
				delete [] _data;
			}
			_data = data2;

			_capacity = num_elements;

			return true;
		}

		virtual void insert_back(type const& object) {
			if (size==_capacity) reserve(size+1);

			*(reinterpret_cast<type*>(_data) + size++) = object;
		}
		type& remove_back(void) {
			assert_term(size>0,"Tried to pop an empty vector!");

			return reinterpret_cast<type*>(_data)[--size];
		}

		inline Vector<type>& operator<<=(type& other) { insert_back(other); return *this; }
		inline Vector<type>& operator>>=(type& other) { other=remove_back(); return *this; }

		inline type&       operator[](size_t index)       {
			assert_term(static_cast<int>(index)<size,"Tried to access a vector (size %d) out of bounds (index %d)!",size,static_cast<int>(index));
			return reinterpret_cast<type*>(_data)[index];
		}
		inline type const& operator[](size_t index) const {
			assert_term(static_cast<int>(index)<size,"Tried to access a vector (size %d) out of bounds (index %d)!",size,static_cast<int>(index));
			return reinterpret_cast<type*>(_data)[index];
		}
};


}
