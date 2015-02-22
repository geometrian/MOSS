#pragma once

#include "../mossc/cstring"


namespace MOSST {


//Note not final; useful for e.g. String to subclass.
template <typename type> class Vector {
	protected:
		unsigned char* _data;
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
		inline virtual ~Vector(void) {
			if (_data!=nullptr) delete [] _data;
		}

		bool reserve(int num_elements) {
			if (_capacity==num_elements) return true;

			unsigned char* data2 = new unsigned char[num_elements*sizeof(type)];
			if (_data!=nullptr) {
				if (size>0) {
					//ASSERT(size<=num_elements,"Tried to resize a vector smaller than the number of elements it contains!");
					if (num_elements<size) return false;
					MOSSC::memcpy(data2,_data, size*sizeof(type));
				}
				delete [] _data;
			}
			_data = data2;

			_capacity = num_elements;

			return true;
		}

		virtual void insert_back(const type& object) {
			if (size==_capacity) {
				if (_capacity==0) {
					reserve(1);
				} else {
					reserve(size+1);
				}
			}
			*( ((type*)(_data)) + size ) = object;
			++size;
		}
		type& remove_back(void) {
			//ASSERT(size>0,"Tried to pop an empty vector!");
			//if (size==0) return;
			return ((type*)(_data))[--size];
		}

		inline Vector<type>& operator<<=(type& other) {
			insert_back(other);
			return *this;
		}
		inline Vector<type>& operator>>=(type& other) {
			other = remove_back();
			return *this;
		}

		inline type& operator[](size_t index) {
			//ASSERT(index<size,"Tried to access a vector out of bounds!");
			//if (index>=size) return;
			return ((type*)(_data))[index];
		}
		inline const type& operator[](size_t index) const {
			//ASSERT(index<size,"Tried to access a vector out of bounds!");
			//if (index>=size) return;
			return ((type*)(_data))[index];
		}
};


}