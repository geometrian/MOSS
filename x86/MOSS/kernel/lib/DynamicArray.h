#pragma once

#include "../memory/memory.h"

template <typename type_element> class DynamicArray {
	protected:
		type_element* data;
		int capacity;
	public:
		int length;
	public:
		DynamicArray(int reserve=0) {
			if (reserve!=0) {
				data = malloc(reserve*sizeof(type_element));
			}
			length = 0;
			capacity = reserve;
		}
		~DynamicArray(void) {
			free(data);
		}

		inline type_elements& operator[] (int  index)       { return data[index]; }
		inline type_elements  operator[] (int  index) const { return data[index]; }

		void add(type_element element) {
			if (length==capacity) {
				int new_size = capacity + 10;

				type_element* data2 = malloc(new_size*sizeof(type_element));
				for (int i=0;i<length;++i) {
					data2[i] = data[i];
				}
				free(data);
				data = data2;

				capacity = new_size;
			}

			data[length++] = element;
		}
		void empty(void) {
			length = 0;
		}

		DynamicArray<DynamicArray<type_element>> split(const DynamicArray<type_element>& separator) const {
			DynamicArray<DynamicArray<type_element>> result;

			DynamicArray<type_element> part;
			for (int i=0;i<length;++i) {
				DynamicArray<type_element>& at = (*this)[i];
				if (at==separator) {
					if (part.length!=0) result.add(part);
					part.empty();
					continue;
				}
				part.add(at);
			}
			if (part.length!=0) result.add(part);

			return result;
		}
};