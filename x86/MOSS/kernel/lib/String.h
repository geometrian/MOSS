#pragma once

#include "clib.h"
#include "DynamicArray.h"

typedef DynamicArray<char> String;

class String : public DynamicArray<char> {
	public:
		String(int reserve=0) : DynamicArray<char>(reserve) {}
		String(const String& copy) : DynamicArray<char>(copy.length) {
			for (int i=0;i<length;++i) {
				(*this)[i] = copy[i];
			}
		}
		String(const char* str) : DynamicArray<char>(strlen(str)) {}
		~String(void) {}

		String& operator=(const char* str) const {
			return String(str);
		}

		const char* as_cstr(void) const {
			return data;
		}

		bool contains(const String& other) const {
			if (other.length>length) return false;

			int max_offset = length - other.length;
			for (int start=0;start<max_offset;++start) {
				for (int i=0;i<other.length;++i) {
					if ((*this)[start+i] != other[i]) goto NEXT;
				}
				return true;
				NEXT:
			}

			return false;
		}
};