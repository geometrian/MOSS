#pragma once

/*
https://en.cppreference.com/w/cpp/header/cctype
COMPLETE except for locale in `tolower(⋯)` and `toupper(⋯)`
*/

#include "__arch.h"



#ifdef __MOSS_CPP__
namespace std
{
	extern "C"
	{
#endif



/* '0'–'9' */
MOSS_ND_CEXPR int isdigit( int c )
{
	union { int i; unsigned char uc; } tmp;
	tmp.i = c;
	tmp.uc -= '0'; /* note overflow */
	return tmp.uc < 10;
}
/* '0'–'9', 'a'–'f', 'A'–'F' */
MOSS_ND_CEXPR int isxdigit( int c )
{
	union { int i; unsigned char uc; } tmp;
	tmp.i = c;
	tmp.uc -= 48; /* note overflow */
	if (tmp.uc>55) tmp.uc=55; /* branchless */
	/* 0b00000000'01111110'00000000'00000000'00000000'01111110'00000011'11111111ull */
	return ( 0x007E0000007E03FFull & (1ull<<tmp.uc) ) != 0;
}

/* 'a'–'z' */
MOSS_ND_CEXPR int islower( int c )
{
	union { int i; unsigned char uc; } tmp;
	tmp.i = c;
	tmp.uc -= 'a'; /* note overflow */
	return tmp.uc < 26;
}
/* 'A'–'Z' */
MOSS_ND_CEXPR int isupper( int c )
{
	union { int i; unsigned char uc; } tmp;
	tmp.i = c;
	tmp.uc -= 'A'; /* note overflow */
	return tmp.uc < 26;
}
/* 'a'–'z', 'A'–'Z' */
MOSS_ND_CEXPR int isalpha( int c )
{
	/* TODO: faster by specializing */
	return islower(c) || isupper(c);
}

/* 'a'–'z', 'A'–'z', '0'–'9' */
MOSS_ND_CEXPR int isalnum( int c )
{
	/* TODO: faster by specializing */
	return isdigit(c) || isalpha(c);
}

/* ' ', '\t' */
MOSS_ND_CEXPR int isblank( int c )
{
	return c==' ' || c=='\t';
}
/* ' ', '\t', '\r', '\n', '\f', '\v' */
MOSS_ND_CEXPR int isspace( int c )
{
	return c==' ' || ( c>=9 && c<=13 );
}

/* (characters that control, instead of print) */
MOSS_ND_CEXPR int iscntrl( int c )
{
	return c<=31 || c==127;
}
/* (characters that print, instead of control) */
MOSS_ND_CEXPR int isprint( int c )
{
	/* characters 32–126 */
	union { int i; unsigned char uc; } tmp;
	tmp.i = c;
	tmp.uc -= 32; /* note overflow */
	return tmp.uc <= (126-32);
}
/* (characters that print, instead of control, exclusive of ' ') */
MOSS_ND_CEXPR int isgraph( int c )
{
	/* characters 33–126 */
	union { int i; unsigned char uc; } tmp;
	tmp.i = c;
	tmp.uc -= 33; /* note overflow */
	return tmp.uc <= (126-33);
}
/* (characters that print, instead of control, exclusive of alphanumeric) */
MOSS_ND_CEXPR int ispunct( int c )
{
	/* TODO: better */
	return
		( c>= 33 && c<= 47 ) ||
		( c>= 58 && c<= 64 ) ||
		( c>= 91 && c<= 96 ) ||
		( c>=123 && c<=126 )
	;
}

/* TODO: locale for these functions */
MOSS_ND_CEXPR int tolower( int c )
{
	if (isupper(c)) c+=32;
	return c;
}
MOSS_ND_CEXPR int toupper( int c )
{
	if (islower(c)) c-=32;
	return c;
}



#ifdef __MOSS_CPP__
	}
}
#endif
