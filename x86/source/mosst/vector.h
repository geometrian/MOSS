#pragma once

#include "../mossc/cstring"


namespace MOSST {


template <typename type> class Vector {
	protected:
		unsigned char* data;
		int capacity;
	public:
		int size;

	public:
		Vector(void) {
			data = NULL;
			capacity = 0;
			size = 0;
		}
		virtual ~Vector(void) {
			delete [] data;
		}

		bool reserve(int num_elements) {
			if (capacity==num_elements) return true;

			unsigned char* data2 = new unsigned char[num_elements*sizeof(type)];
			if (data!=NULL) {
				if (size>0) {
					//ASSERT(size<=num_elements,"Tried to resize a vector smaller than the number of elements it contains!");
					if (num_elements<size) return false;
					MOSSC::memcpy(data2,data, size*sizeof(type));
				}
				delete [] data;
			}
			data = data2;

			capacity = num_elements;

			return true;
		}

		virtual void insert_back(const type& object) {
			if (size==capacity) {
				if (capacity==0) {
					reserve(1);
				} else {
					reserve(size+1);
				}
			}
			*( ((type*)(data)) + size ) = object;
			++size;
		}
		type& remove_back(void) {
			//ASSERT(size>0,"Tried to pop an empty vector!");
			//if (size==0) return;
			return ((type*)(data))[--size];
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
			return ((type*)(data))[index];
		}
		inline const type& operator[](size_t index) const {
			//ASSERT(index<size,"Tried to access a vector out of bounds!");
			//if (index>=size) return;
			return ((type*)(data))[index];
		}
};


}