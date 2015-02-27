#pragma once

#include "../includes.h"

#include "vector.h"


namespace MOSST {


class String final : public Vector<char> {
	public:
		inline String(void) : Vector() {}
		String(char const* data); //Note not explicit
		inline virtual ~String(void) {}

		void insert_back(char const& object) override;

		String operator+(String const& other);

		String& operator+=(char const* other);
		String& operator+=(String const& other);

		char const* c_str(void) const;
};


}
