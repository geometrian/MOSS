#pragma once

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

//#include "../string/copying.h"


#define TEMP_BUFFER_WRITE(FUNCTION)\
	char temp[64]={};\
	int num_written;\
	{\
		char* temp2=temp;\
		temp2 = FUNCTION(temp2, value);\
		if (temp2==NULL) return NULL;\
		num_written = temp2 - temp;\
	}

class SpecifierBase {
	protected:
		int min_width;
		int precision;

		bool flag_ljst;
		int  flag_sign;
		bool flag_hash;
		bool flag_zero;

		int len_conv;

		va_list*const args;

	public:
		bool valid;

	protected:
		SpecifierBase(va_list* args, const char* specifier_str,int specifier_length) : args(args) {
			valid = true;
			#define FAIL { valid=false; return; }

			//specifier_str looks like "%[flags][width][.precision][length]specifier"
			const char* expected_end = specifier_str + specifier_length;

			//Skip %
			++specifier_str;

			//Parse flags
			specifier_str = parse_flags(specifier_str);
			if (specifier_str==NULL) FAIL
			if (flag_hash) FAIL //TODO: not implemented!

			//Parse width
			bool got_width;
			specifier_str = parse_numorstar(specifier_str, &min_width,&got_width);
			if (got_width) {
				if (min_width<0) FAIL //This can happen with the *.
			} else {
				min_width = 0;
			}

			//Parse precision
			if (*specifier_str=='.') { //precision expected
				bool got_precision;
				specifier_str = parse_numorstar(specifier_str+1, &precision,&got_precision);
				if (got_precision) {
					if (precision<0) FAIL //This can happen with the *.
				} else { //There was nothing following the ".".
					precision = 0;
				}
			} else {
				precision = -1;
			}

			//Parse length conversion
			specifier_str = parse_length(specifier_str);
			if (len_conv!=0) FAIL //TODO: not implemented!

			//Get rid of the specifier
			++specifier_str;

			//Check that we got everything
			if (specifier_str!=expected_end) FAIL

			#undef FAIL
		}
	public:
		virtual ~SpecifierBase(void) {}

	public:
		virtual char* write(char* buffer) = 0;

	private:
		const char* parse_flags(const char* specifier_str) {
			flag_ljst = false;
			flag_sign =     0;
			flag_hash = false;
			flag_zero = false;
			bool found_ljst = false;
			bool found_sign = false;
			bool found_hash = false;
			bool found_zero = false;
			LOOP:
				switch (*specifier_str) {
					case '-':
						if (found_ljst) return NULL; found_ljst=true;
						flag_ljst = true;
						break;
					case '+':
						if (found_sign) return NULL; found_sign=true;
						flag_sign = 1;
						break;
					case ' ':
						if (found_sign) return NULL; found_sign=true;
						flag_sign = 2;
						break;
					case '#':
						if (found_hash) return NULL; found_hash=true;
						flag_hash = true;
						break;
					case '0':
						if (found_zero) return NULL; found_zero=true;
						flag_zero = true;
						break;
					default:
						return specifier_str;
				}
				++specifier_str;
				goto LOOP;
		}
		const char* parse_numorstar(const char* specifier_str, int* num,bool* got_num) {
			if (*specifier_str=='*') {
				*num = va_arg(*args,int);
				*got_num = true;
				return specifier_str + 1;
			}

			*num = 0;
			*got_num = false;
			LOOP:
				char c = *specifier_str;
				switch (c) {
					case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
						*num *= 10;
						*num += (int)(c-'0');
						*got_num = true;
						break;
					default:
						return specifier_str;
				}
				++specifier_str;
				goto LOOP;
		}
		const char* parse_length(const char* specifier_str) {
			switch (specifier_str[0]) {
				case 'h':
					switch (specifier_str[1]) {
						case 'h': len_conv=1; return specifier_str;
						default: len_conv=2; return specifier_str;
					}
				case 'l':
					switch (specifier_str[1]) {
						case 'l': len_conv=4; return specifier_str;
						default: len_conv=3; return specifier_str;
					}
				case 'j': len_conv=5; return specifier_str;
				case 'z': len_conv=6; return specifier_str;
				case 't': len_conv=7; return specifier_str;
				case 'L': len_conv=8; return specifier_str;
				default: len_conv=0; return specifier_str;
			}
		}

	protected:
		//Anywthing written still counts against the minimum number of characters we need to write
		//When writing, do this first, since any padding should happen after the sign.
		//Also makes value unsigned, if it wasn't already.
		char* write_sign_u(char* buffer) {
			switch (flag_sign) {
				case 0: break; //default
				case 1:
					*(buffer++) = '+';
					--min_width;
					break;
				case 2:
					*(buffer++) = ' ';
					--min_width;
					break;
			}
			return buffer;
		}
		template <typename type_signed> char* write_sign_s(char* buffer, type_signed* value) {
			if (*value>=0) {
				buffer = write_sign_u(buffer);
			} else {
				*(buffer++) = '-';
				*value = -*value;
				--min_width;
			}
			return buffer;
		}

		char* write_data(char* buffer, const char* temp,int temp_length) {
			//Actually write the type!
			//	The algorithm is to write into a temporary buffer, then copy it into the real string.  This allows for
			//	padding to take place, as necessary.  Since the way it gets put into the temporary buffer may be unique
			//	to each type, subclasses call this method after having created the temporary buffer.

			if (flag_ljst) {
				//Copy the string over directly, and then pad the rest with spaces
				if (flag_zero) return NULL; //must pad with spaces
				int i = 0;
				for (;i<temp_length;++i) {
					*(buffer++) = temp[i];
				}
				for (;i<min_width;++i) {
					*(buffer++) = ' ';
				}
			} else {
				int num_pad = min_width - temp_length;
				if (flag_zero) for (int i=0;i<num_pad;++i) *(buffer++)='0';
				else           for (int i=0;i<num_pad;++i) *(buffer++)=' ';
				for (int i=0;i<temp_length;++i) *(buffer++) = temp[i];
			}

			return buffer;
		}

		char* write_float(char* str, double value) {
			double larger = 1.0;
			while (larger<value) {

			}
			return str;
		}
};
class SpecifierIntegralBase : public SpecifierBase {
	protected:
		SpecifierIntegralBase(va_list* args, const char* specifier_str,int specifier_length) : SpecifierBase(args, specifier_str,specifier_length) {}
	public:
		virtual ~SpecifierIntegralBase(void) {}

	protected:
		char* write_uint(char* str, uint64_t value) {
			if (precision==0 && value==0ull) return str;

			//largest is 18446744073709551615 => 20 characters long
			char temp[20];
			int i = 0;
			LOOP:
				unsigned char digit = (unsigned char)(value%10ull);
				value /= 10ull;
				temp[i] = digit + '0';
				if (value>0) {
					++i;
					goto LOOP;
				}

			int num_zeros = precision - (i+1);
			for (int j=0;j<num_zeros;++j) {
				*(str++) = '0';
			}

			while (i>=0) {
				*(str++) = temp[i--];
			}
			return str;
		}
};
class SpecifierSignedIntegral : public SpecifierIntegralBase {
	public:
		SpecifierSignedIntegral(va_list* args, const char* specifier_str,int specifier_length) : SpecifierIntegralBase(args, specifier_str,specifier_length) {}
		virtual ~SpecifierSignedIntegral(void) {}

		char* write(char* buffer) override {
			int value = va_arg(*args,int);
			buffer = write_sign_s(buffer,&value);
			TEMP_BUFFER_WRITE(write_uint)
			return write_data(buffer, temp,num_written);
		}
};
class SpecifierUnsignedIntegral : public SpecifierIntegralBase {
	public:
		SpecifierUnsignedIntegral(va_list* args, const char* specifier_str,int specifier_length) : SpecifierIntegralBase(args, specifier_str,specifier_length) {}
		~SpecifierUnsignedIntegral(void) {}

		char* write(char* buffer) override {
			unsigned int value = va_arg(*args,unsigned int);
			buffer = write_sign_u(buffer);
			TEMP_BUFFER_WRITE(write_uint)
			return write_data(buffer, temp,num_written);
		}
};
class SpecifierCharacter : public SpecifierBase {
	public:
		SpecifierCharacter(va_list* args, const char* specifier_str,int specifier_length) : SpecifierBase(args, specifier_str,specifier_length) {
			if (flag_sign!=0) valid=false;
			if (precision!=-1) valid=false;
		}
		~SpecifierCharacter(void) {}

		char* write(char* buffer) override {
			char value = va_arg(*args,char);
			TEMP_BUFFER_WRITE(write_char)
			return write_data(buffer, temp,num_written);
		}

	private:
		char* write_char(char* buffer, char value) {
			*(buffer++) = value;
			return buffer;
		}
};
class SpecifierString : public SpecifierBase {
	public:
		SpecifierString(va_list* args, const char* specifier_str,int specifier_length) : SpecifierBase(args, specifier_str,specifier_length) {
			if (flag_sign!=0) valid=false;
			if (precision!=-1) valid=false;
		}
		virtual ~SpecifierString(void) {}

		char* write(char* buffer) override {
			char* value = va_arg(*args,char*);
			return write_data(buffer, value,strlen(value));
		}

	private:
		char* write_string(char* str, char* value) {
			LOOP:
				char c = *value;
				if (c!='\0') {
					*(str++) = c;
					++value;
					goto LOOP;
				}
			return str;
		}
};

static void write_fmtspecifier_error(char* str, char specifier) {
	char message[] = {'<','e','r','r','o','r',' ','\'',specifier,'\'','>'};
	size_t i=0u, n=sizeof(message)/sizeof(char);
	do {
		str[i] = message[i];
	} while (++i!=n);
}
static char* write_fmtspecifier(char* str, const char* specifier_str,int specifier_length, va_list* args) {
	//specifier_str looks like "%[flags][width][.precision][length]specifier"

	char specifier_ch = specifier_str[specifier_length-1];

	SpecifierBase* specifier = NULL;
	switch (specifier_ch) {
		case 'd': //Fallthrough
		case 'i': specifier=new   SpecifierSignedIntegral(args, specifier_str,specifier_length); break; //Signed decimal
		case 'u': specifier=new SpecifierUnsignedIntegral(args, specifier_str,specifier_length); break; //Unsigned decimal
		case 'o': break; //Unsigned octal
		case 'x': break; //Unsigned hex
		case 'X': break; //Unsigned hex (uppercase)
		case 'f': //Fallthrough TODO: I don't know what the difference is supposed to be! //Floating-point
		//case 'F': return SpecifierFloat(args, specifier_str,specifier_length).write(str); //Floating-point (uppercase)
		case 'e': break; //Scientific notation
		case 'E': break; //Scientific notation (uppercase)
		case 'g': break; //Shortest of using f xor e
		case 'G': break; //Shortest of using F xor E
		case 'a': break; //Floating-point hex
		case 'A': break; //Floating-point hex (uppercase)
		case 'c': specifier=new SpecifierCharacter(args, specifier_str,specifier_length); break; //Character
		case 's': specifier=new SpecifierString(args, specifier_str,specifier_length); break; //String
		case 'p': break; //Pointer
		case 'n': break; //Writes number of characters written
		case '%': //Write %
			if (specifier_length!=2) break;
			*str = '%';
			return str + 1;
		default: break;
	}

	if (specifier==NULL) goto PARSE_ERROR;
	if (!specifier->valid) {
		delete specifier;
		goto PARSE_ERROR;
	}

	str = specifier->write(str);
	delete specifier;
	return str;

	PARSE_ERROR:
		write_fmtspecifier_error(str, specifier_ch);
		return NULL;
}

static bool string_contains(const char* string, char character) {
	int i = 0;
	LOOP:
		char c = string[i];
		if (c==character) return true;
		if (c==     '\0') return false;
		++i;
		goto LOOP;
}

int sprintf(char* str, const char* format, ...) {
	int return_value = 0;
	char* str_original = str;

	va_list args;
	va_start(args,format);

	LOOP1:
		char c1 = format[0];

		if (c1=='%') {
			const static char* specifiers = "diuoxXfFeEgGaAcspn%";
			int i = 1;
			LOOP2:
				char c2 = format[i];
				if (c2=='\0') {
					write_fmtspecifier_error(str,'?');
					return_value = -1; //TODO: wrong number?
					goto END;
				}
				if (string_contains(specifiers,c2)) {
					str = write_fmtspecifier(str, format,i+1, &args);
					if (str==NULL) {
						return_value = -1; //TODO: wrong number?
						goto END;
					}
					format += i + 1;
					goto LOOP1;
				} else {
					++i;
					goto LOOP2;
				}
		} else {
			*(str++) = c1;
		}

		if (c1!='\0') {
			++format;
			goto LOOP1;
		}
	END:

	va_end(args);

	if (return_value==0) {
		return_value = str - str_original - 1;
	}
	return return_value;
}