#include "string.h"


namespace MOSST {


String::String(void) : Vector() {
	push_back('\0');
}
String::String(const char* data) : Vector() {
	push_back('\0');
	*this += data;
}
String::~String(void) {
}

int String::get_size(void) const /*override*/ {
	return size - 1; //Do not count the NULL character.
}

String String::operator+(const String& other) {
	String result(this->c_str());
	result += other.c_str();
	return result;
}

String& String::operator+=(const char* other) {
	pop_back();
	int i = 0;
	LOOP:
		char c = other[i];
		push_back(c);
		if (c!='\0') {
			++i;
			goto LOOP;
		}
	return *this;
}
String& String::operator+=(const String& other) {
	*this += other.c_str();
	return *this;
}

const char* String::c_str(void) const {
	return (const char*)(this->data);
}


}