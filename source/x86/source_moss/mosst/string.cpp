#include "string.h"


namespace MOSST {


String::String(void) : Vector() {
}
String::String(const char* data) : Vector() {
	*this += data;
}
String::~String(void) {
}

void String::insert_back(const char& object) /*override*/ {
	Vector::insert_back(object);

	//Push a NULL character onto the string, but pretend it doesn't exist.
	Vector::insert_back('\0');
	--size;
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
			insert_back(c);
			++i;
			goto LOOP;
		}
	return *this;
}
String& String::operator+=(const String& other) {
	for (int i=0;i<other.size;++i) {
		insert_back(other[i]);
	}
	return *this;
}

const char* String::c_str(void) const {
	return (const char*)(this->data);
}


}