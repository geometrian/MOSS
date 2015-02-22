#include "string.h"


namespace MOSST {


String::String(void) : Vector() {}
String::String(const char* data) : Vector() {
	*this += data;
}
String::~String(void) {
}

String String::operator+(const String& other) {
	String result(this->c_str());
	result += other.c_str();
	return result;
}

String& String::operator+=(const char* other) {
	int i = 0;
	LOOP:
		char c = other[i];
		if (c!='\0') {
			push_back(c);
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