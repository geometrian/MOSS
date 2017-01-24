#pragma once

#include "../includes.hpp"

#include "vector.hpp"


namespace MOSST {


class String final : public Vector<char> {
	public:
		inline String(void) : Vector() {}
		String(char const* data); //Note not explicit
		inline virtual ~String(void) = default;

		String& operator=(String const& other);

		virtual void insert_back(char const& object) override;

		String operator+(String const& other);

		String& operator+=(char const* other);
		String& operator+=(String const& other);

		bool operator==(String const& other) const;

		char const* c_str(void) const;

	private:
		void _fix_null(void);
};


}
