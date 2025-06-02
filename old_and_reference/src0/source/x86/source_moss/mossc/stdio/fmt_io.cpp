#include "fmt_io.hpp"

#include "../string/copying.hpp"
#include "../string/other.hpp"


namespace MOSSC {


#define INTEGRAL_BUFFER_SIZE 20
#define FLOAT_BUFFER_SIZE 256 //TODO: the float function may not be entirely safe about using this!
#define TEMP_BUFFER_WRITE(SIZE,FUNCTION)\
	char temp[SIZE]={};\
	uint64_t num_written;\
	{\
		char* temp2=temp;\
		temp2 = FUNCTION(temp2, value);\
		if (temp2==nullptr) return nullptr;\
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
		SpecifierBase(va_list* args, char const* specifier_str,int specifier_length) : args(args) {
			valid = true;
			#define FAIL { valid=false; return; }

			//specifier_str looks like "%[flags][width][.precision][length]specifier"
			char const* expected_end = specifier_str + specifier_length;

			//Skip %
			++specifier_str;

			//Parse flags
			specifier_str = parse_flags(specifier_str);
			if (specifier_str==nullptr) FAIL

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
		inline virtual ~SpecifierBase(void) = default;

	public:
		virtual char* write(char* buffer) = 0;

	private:
		char const* parse_flags(char const* specifier_str) {
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
						if (found_ljst) return nullptr;
						found_ljst = true;
						flag_ljst  = true;
						break;
					case '+':
						if (found_sign) return nullptr;
						found_sign = true;
						flag_sign  =    1;
						break;
					case ' ':
						if (found_sign) return nullptr;
						found_sign = true;
						flag_sign  =    2;
						break;
					case '#':
						if (found_hash) return nullptr;
						found_hash = true;
						flag_hash  = true;
						break;
					case '0':
						if (found_zero) return nullptr;
						found_zero = true;
						flag_zero  = true;
						break;
					default:
						return specifier_str;
				}
				++specifier_str;
				goto LOOP;
		}
		char const* parse_numorstar(char const* specifier_str, int* num,bool* got_num) {
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
		char const* parse_length(char const* specifier_str) {
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
		int get_float_kind(double value) const {
			// 0 = ordinary double
			//-1 = -INF
			// 1 = +INF
			// 2 = NaN

			//http://en.wikipedia.org/wiki/Double-precision_floating-point_format
			union {
				  double d;
				uint64_t i;
			} convert;
			convert.d = value;

			if (convert.i==0x7FF0000000000000) { //+INF
				return  1;
			} else if (convert.i==0xFFF0000000000000) { //-INF
				return -1;
			} else if ((convert.i&0x7FF0000000000000ull)==0x7FF0000000000000ull && convert.i&0x000FFFFFFFFFFFFFull) { //NaN
				return  2;
			}
			return 0;
		}

		//Anything written still counts against the minimum number of characters we need to write
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
			if (*value>=(type_signed)(0)) {
				buffer = write_sign_u(buffer);
			} else {
				*(buffer++) = '-';
				*value = -*value;
				--min_width;
			}
			return buffer;
		}
		char* write_sign_f(char* buffer, double* value) {
			if (*value>=0.0 || get_float_kind(*value)==2) { //NaNs are erroneously reported negative
				buffer = write_sign_u(buffer);
			} else {
				*(buffer++) = '-';
				*value = -*value;
				--min_width;
			}
			return buffer;
		}

		char* write_data(char* buffer, char const* temp,uint64_t temp_length) const {
			//Actually write the type!
			//	The algorithm is to write into a temporary buffer, then copy it into the real string.  This allows for
			//	padding to take place, as necessary.  Since the way it gets put into the temporary buffer may be unique
			//	to each type, subclasses call this method after having created the temporary buffer.

			if (flag_ljst) {
				//Copy the string over directly, and then pad the rest with spaces
				if (flag_zero) return nullptr; //must pad with spaces
				int i = 0;
				for (;i<(int)(temp_length);++i) {
					*(buffer++) = temp[i];
				}
				for (;i<min_width;++i) {
					*(buffer++) = ' ';
				}
			} else {
				int num_pad = (int)(min_width - temp_length); //can be negative
				if (flag_zero) for (int i=0;i<num_pad;++i) *(buffer++)='0';
				else           for (int i=0;i<num_pad;++i) *(buffer++)=' ';
				for (int i=0;i<(int)(temp_length);++i) *(buffer++) = temp[i];
			}

			return buffer;
		}
};
class SpecifierIntegralBase : public SpecifierBase {
	protected:
		SpecifierIntegralBase(va_list* args, char const* specifier_str,int specifier_length) : SpecifierBase(args, specifier_str,specifier_length) {
			if (precision>20) valid=false; //That's larger than any integer, and would cause a problem with the 20 char buffer in .write_uint(...)!
		}
	public:
		inline virtual ~SpecifierIntegralBase(void) = default;

	protected:
		char* write_udec(char* buffer, uint64_t value) const {
			if (precision==0 && value==0ull) return buffer;

			//largest is 18446744073709551615 => 20 characters long
			char temp[20];
			int i = 0;
			LOOP:
				uint8_t digit=static_cast<uint8_t>(value%10ull); value/=10ull;
				temp[i] = digit + '0';
				if (value>0) {
					++i;
					goto LOOP;
				}

			int num_zeros = precision - (i+1);
			for (int j=0;j<num_zeros;++j) *(buffer++)='0';

			while (i>=0) *(buffer++)=temp[i--];

			return buffer;
		}
		char* write_uhex(char* buffer, uint64_t value) const {
			if (precision==0 && value==0ull) return buffer;

			//largest is FFFFFFFFFFFFFFFF => 16 characters long
			char temp[16];
			int i = 0;
			LOOP:
				uint8_t hexit=static_cast<uint8_t>(value&0x0Full); value>>=4;
				if (hexit<10u) temp[i]=hexit   +'0';
				else           temp[i]=hexit-10+'A';

				if (value>0) {
					++i;
					goto LOOP;
				}

			int num_zeros = precision - (i+1);
			for (int j=0;j<num_zeros;++j) *(buffer++)='0';

			while (i>=0) *(buffer++)=temp[i--];

			return buffer;
		}
};
class SpecifierSignedDec final : public SpecifierIntegralBase {
	public:
		SpecifierSignedDec(va_list* args, char const* specifier_str,int specifier_length) : SpecifierIntegralBase(args, specifier_str,specifier_length) {
			if (flag_hash) valid=false;
		}
		inline virtual ~SpecifierSignedDec(void) = default;

		char* write(char* buffer) override {
			int value = va_arg(*args,int);
			buffer = write_sign_s(buffer,&value);
			TEMP_BUFFER_WRITE(INTEGRAL_BUFFER_SIZE,write_udec)
			return write_data(buffer, temp,num_written);
		}
};
class SpecifierUnsignedDec final : public SpecifierIntegralBase {
	public:
		SpecifierUnsignedDec(va_list* args, char const* specifier_str,int specifier_length) : SpecifierIntegralBase(args, specifier_str,specifier_length) {
			if (flag_hash) valid=false;
		}
		inline virtual ~SpecifierUnsignedDec(void) = default;

		char* write(char* buffer) override {
			unsigned int value = va_arg(*args,unsigned int);
			buffer = write_sign_u(buffer);
			TEMP_BUFFER_WRITE(INTEGRAL_BUFFER_SIZE,write_udec)
			return write_data(buffer, temp,num_written);
		}
};
class SpecifierUnsignedHex final : public SpecifierIntegralBase {
	public:
		SpecifierUnsignedHex(va_list* args, char const* specifier_str,int specifier_length) : SpecifierIntegralBase(args, specifier_str,specifier_length) {
			if (flag_hash) valid=false;
		}
		inline virtual ~SpecifierUnsignedHex(void) = default;

		char* write(char* buffer) override {
			unsigned int value = va_arg(*args,unsigned int);
			buffer = write_sign_u(buffer);
			TEMP_BUFFER_WRITE(INTEGRAL_BUFFER_SIZE,write_uhex)
			return write_data(buffer, temp,num_written);
		}
};
class SpecifierPointer final : public SpecifierIntegralBase {
	public:
		SpecifierPointer(va_list* args, char const* specifier_str,int specifier_length) : SpecifierIntegralBase(args, specifier_str,specifier_length) {
			if (flag_sign!=0) valid=false;
			else if (flag_hash) valid=false;
			else if (precision!=-1) valid=false;
		}
		inline virtual ~SpecifierPointer(void) = default;

		char* write(char* buffer) override {
			void* value = va_arg(*args,void*);
			TEMP_BUFFER_WRITE(16,write_pointer)
			return write_data(buffer, temp,num_written);
		}

	private:
		char* write_pointer(char* buffer, void* value) {
			uint64_t value2 = (uint64_t)(value);

			int bits = static_cast<int>(sizeof(void*)) * 8;

			for (int i=bits-4;i>=0;i-=4) {
				uint8_t hexit = static_cast<uint8_t>((value2>>i)&0x0F);
				if (hexit<10u) *(buffer++)=hexit   +'0';
				else           *(buffer++)=hexit-10+'A';
			}

			return buffer;
		}
};
class SpecifierCharacter final : public SpecifierBase {
	public:
		SpecifierCharacter(va_list* args, char const* specifier_str,int specifier_length) : SpecifierBase(args, specifier_str,specifier_length) {
			if (flag_sign!=0) valid=false;
			else if (flag_hash) valid=false;
			else if (precision!=-1) valid=false;
		}
		inline virtual ~SpecifierCharacter(void) = default;

		char* write(char* buffer) override {
			char value = (int)(va_arg(*args,int));
			TEMP_BUFFER_WRITE(1,write_char)
			return write_data(buffer, temp,num_written);
		}

	private:
		char* write_char(char* buffer, char value) {
			*(buffer++) = value;
			return buffer;
		}
};
class SpecifierString final : public SpecifierBase {
	public:
		SpecifierString(va_list* args, char const* specifier_str,int specifier_length) : SpecifierBase(args, specifier_str,specifier_length) {
			if (flag_sign!=0) valid=false;
			else if (flag_hash) valid=false;
			else if (precision!=-1) valid=false;
		}
		inline virtual ~SpecifierString(void) = default;

		char* write(char* buffer) override {
			char* value = va_arg(*args,char*);
			return write_data(buffer, value,strlen(value));
		}

	private:
		char* write_string(char* buffer, char* value) {
			LOOP:
				char c = *value;
				if (c!='\0') {
					*(buffer++) = c;
					++value;
					goto LOOP;
				}
			return buffer;
		}
};
class SpecifierFloat final : public SpecifierBase {
	public:
		SpecifierFloat(va_list* args, char const* specifier_str,int specifier_length) : SpecifierBase(args, specifier_str,specifier_length) {
			if (precision==-1) precision = 6;
		}
		inline virtual ~SpecifierFloat(void) = default;

		char* write(char* buffer) override {
			double value = va_arg(*args,double);
			buffer = write_sign_f(buffer,&value);
			TEMP_BUFFER_WRITE(FLOAT_BUFFER_SIZE,write_float)
			return write_data(buffer, temp,num_written);
		}

	private:
		inline char* write_float_high(char* buffer, double* value) {
			if (*value>=1.0) {
				double larger1 = 1.0;
				double larger2 = 0.1;
				while (larger1<=*value) {
					larger2 *= 10.0;
					larger1 *= 10.0;
				}
				LOOP:
					char digit = '0';
					while (*value>=larger2) {
						++digit;
						*value -= larger2;
					}
					if (*value>1.0) {
						*(buffer++) = digit;
						larger1 /= 10.0;
						larger2 /= 10.0;
						goto LOOP;
					} else {
						if (precision==0 && *value>=0.5) ++digit; //rounding
						*(buffer++) = digit;
					}
			} else {
				*(buffer++) = (precision==0 && *value>=0.5) ? '1' : '0'; //rounding
			}
			return buffer;
		}
		inline char* write_float_low(char* buffer, double value) {
			if (precision==0) return buffer; //It's not hard to handle this later, but it allows us to make the loop bottom-tested, which is faster and more intuitive

			int i = 0;
			LOOP:
				char digit = '0';
				value *= 10.0;
				while (value>1.0) {
					++digit;
					value -= 1.0;
				}

				if (++i==precision) {
					if (value>=0.5) ++digit; //round
					*(buffer++) = digit;
					return buffer;
				}

				*(buffer++) = digit;

				goto LOOP;
		}
		char* write_float(char* buffer, double value) {
			#define WRITE_STRING(STR) { memmove(buffer,STR,sizeof(STR)-1); buffer+=sizeof(STR)-1; }
			switch (get_float_kind(value)) {
				case  0: {//ordinary double
					char* buffer_orig = buffer;

					//See the comment below for why the +1.
					buffer = write_float_high(buffer+1,&value);
					if (value>0.0 || flag_hash) {
						*(buffer++) = '.';
					}
					buffer = write_float_low(buffer,value);

					uint64_t written = buffer - buffer_orig;

					//The individual functions do rounding, which just increments the least significant digit.
					//If it happened to be '9' however, then it will overflow to ':'.  So, we go back through
					//what we just wrote and check it, propagating the results back if we find any.  This can
					//cause an extra digit to be pushed onto the front, which we must be ready for.

					//To this end, the +1 is already in-place, and we can write something there.  If we didn't
					//overflow, then we shift everything back to cover it up.

					bool carry = false;
					for (uint64_t i=written-1;i>=1;--i) {
						if (buffer_orig[i]=='.') continue;
						if (carry) {
							++(buffer_orig[i]);
							carry = false;
						}
						if (buffer_orig[i]==':') {
							buffer_orig[i] = '0';
							carry = true;
						}
					}
					if (carry) {
						buffer_orig[0] = '1';
					} else {
						//shift everything back
						uint64_t i;
						for (i=1;i<written;++i) {
							buffer_orig[i-1] = buffer_orig[i];
						}
						buffer_orig[written-1] = '\0';
						--buffer;
					}

					break;
				}
				case -1: //-INF (sign has already been written)
					WRITE_STRING("INF")
					break;
				case  1: //+INF (sign has already been written)
					WRITE_STRING("INF")
					break;
				case  2: //NaN
					WRITE_STRING("NaN")
					break;
			}
			#undef WRITE_STRING
			return buffer;
		}
};

static void write_fmtspecifier_error(char* str, char specifier) {
	char message[] = {'<','e','r','r','o','r',' ','\'',specifier,'\'','>','\0'};
	size_t i=0u, n=sizeof(message)/sizeof(char);
	do {
		str[i] = message[i];
	} while (++i!=n);
}
static char* write_fmtspecifier(char* str, char const* specifier_str,int specifier_length, va_list* args) {
	//specifier_str looks like "%[flags][width][.precision][length]specifier"

	char specifier_ch = specifier_str[specifier_length-1];

	SpecifierBase* specifier = nullptr;
	//TODO: the ones that aren't yet supported!
	switch (specifier_ch) {
		case 'd': //Fallthrough
		case 'i': specifier=new   SpecifierSignedDec(args, specifier_str,specifier_length); break; //Signed decimal
		case 'u': specifier=new SpecifierUnsignedDec(args, specifier_str,specifier_length); break; //Unsigned decimal
		case 'o': break; //Unsigned octal
		case 'x': break; //Unsigned hex
		case 'X': specifier=new SpecifierUnsignedHex(args, specifier_str,specifier_length); break; //Unsigned hex (uppercase)
		case 'f': //Fallthrough //TODO: I don't know what the difference is supposed to be! //Floating-point
		case 'F': specifier=new SpecifierFloat(args, specifier_str,specifier_length); break; //Floating-point (uppercase)
		case 'e': break; //Scientific notation
		case 'E': break; //Scientific notation (uppercase)
		case 'g': break; //Shortest of using f xor e
		case 'G': break; //Shortest of using F xor E
		case 'a': break; //Floating-point hex
		case 'A': break; //Floating-point hex (uppercase)
		case 'c': specifier=new SpecifierCharacter(args, specifier_str,specifier_length); break; //Character
		case 's': specifier=new SpecifierString(args, specifier_str,specifier_length); break; //String
		case 'p': specifier=new SpecifierPointer(args, specifier_str,specifier_length); break; //Pointer
		case 'n': break; //Writes number of characters written
		case '%': //Write %
			if (specifier_length!=2) break;
			*str = '%';
			return str + 1;
		default: break;
	}

	if (specifier==nullptr) goto PARSE_ERROR;
	if (!specifier->valid) {
		delete specifier;
		goto PARSE_ERROR;
	}

	str = specifier->write(str);
	delete specifier;
	return str;

	PARSE_ERROR:
		write_fmtspecifier_error(str, specifier_ch);
		return nullptr;
}

static bool string_contains(char const* string, char character) {
	int i = 0;
	LOOP:
		char c = string[i];
		if (c==character) return true;
		if (c==     '\0') return false;
		++i;
		goto LOOP;
}


int sprintf(char* buffer, char const* format, ...) {
	va_list args;
	va_start(args,format);

	int return_value = vsprintf(buffer,format,args);

	va_end(args);

	return return_value;
}

int vsprintf(char* buffer, char const* format, va_list args) {
	int return_value = 0;
	char* buffer_original = buffer;

	LOOP1:
		char c1 = format[0];

		if (c1=='%') {
			static char const* specifiers = "diuoxXfFeEgGaAcspn%";
			int i = 1;
			LOOP2:
				char c2 = format[i];
				if (c2=='\0') {
					write_fmtspecifier_error(buffer,'?');
					return_value = -1; //TODO: wrong number?
					goto END;
				}
				if (string_contains(specifiers,c2)) {
					buffer = write_fmtspecifier(buffer, format,i+1, &args);
					if (buffer==nullptr) {
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
			*(buffer++) = c1;
		}

		if (c1!='\0') {
			++format;
			goto LOOP1;
		}
	END:

	if (return_value==0) {
		return_value = (int)(buffer - buffer_original) - 1;
	}
	return return_value;
}


}
