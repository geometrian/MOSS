#include "../libmoss-stdc/stdio.h"
#include "../libmoss-stdc/string.h"
#include "../libmoss-stdc/__fixpt.h"
#include "../libmoss-stdc/__fprint.h"

#include <float.h>
#include <inttypes.h>
#include <stdbool.h>



static void test_bcd_helper( double val )
{
	_MOSS_FixPtBCD1076 bcd;
	__moss_fixpt_bcd1076_from_f64( &bcd, val );

	if ( bcd.is_neg ) putchar( '-' );
	for ( int k=bcd.digits_count-1; k>=0; --k )
	{
		putchar( '0' + bcd.digits[k] );
		if ( k == bcd.decpt_ind ) putchar('.');
	}
	printf(" (%d)\n", bcd.digits_count );
}
static void test_bcd(void)
{
	test_bcd_helper(    0.0               );
	test_bcd_helper( DBL_TRUE_MIN         );
	test_bcd_helper(    0.00216           );
	test_bcd_helper(    1.0               );
	test_bcd_helper(    3.141592653589793 );
	test_bcd_helper( 1234.0               );
	test_bcd_helper( DBL_MAX              );
}

static void test_fprint_dec_u_helper( uint64_t val, int prec, bool septicks )
{
	char buf[256];
	FILE file_str = __moss_file_str( buf, 256 );

	int n = __moss_fprint_dec_u( &file_str, val, prec, septicks );

	printf( "(prec=%+d,sep=%d)   %" PRIu64 "   ->   \"%s\" (%d,%zu)\n",
		prec,(int)septicks, val, buf, n,strlen(buf)
	);
}
static void test_fprint_dec_u(void)
{
	test_fprint_dec_u_helper(         0, -1, false );
	test_fprint_dec_u_helper(         0,  0, false );
	test_fprint_dec_u_helper(         0,  5, false );
	puts("");
	test_fprint_dec_u_helper(         0, -1, true  );
	test_fprint_dec_u_helper(         0,  0, true  );
	test_fprint_dec_u_helper(         0,  5, true  );
	puts("");

	test_fprint_dec_u_helper(         3, -1, false );
	test_fprint_dec_u_helper(         3,  0, false );
	test_fprint_dec_u_helper(         3,  5, false );
	puts("");
	test_fprint_dec_u_helper(         3, -1, true  );
	test_fprint_dec_u_helper(         3,  0, true  );
	test_fprint_dec_u_helper(         3,  5, true  );
	puts("");

	test_fprint_dec_u_helper(     31415, -1, false );
	test_fprint_dec_u_helper(     31415,  0, false );
	test_fprint_dec_u_helper(     31415,  3, false );
	test_fprint_dec_u_helper(     31415,  8, false );
	puts("");
	test_fprint_dec_u_helper(     31415, -1, true  );
	test_fprint_dec_u_helper(     31415,  0, true  );
	test_fprint_dec_u_helper(     31415,  3, true  );
	test_fprint_dec_u_helper(     31415,  8, true  );
	puts("");

	test_fprint_dec_u_helper( 314159265, -1, false );
	test_fprint_dec_u_helper( 314159265,  0, false );
	test_fprint_dec_u_helper( 314159265,  3, false );
	test_fprint_dec_u_helper( 314159265,  8, false );
	puts("");
	test_fprint_dec_u_helper( 314159265, -1, true  );
	test_fprint_dec_u_helper( 314159265,  0, true  );
	test_fprint_dec_u_helper( 314159265,  3, true  );
	test_fprint_dec_u_helper( 314159265,  8, true  );
}

static void test_fprint_fp_dec_helper( double val, int prec, bool always_pt )
{
	printf(
		always_pt ? "%#.*f\n" : "%.*f\n",
		prec, val
	);
}
static void test_fprint_fp_dec(void)
{
	//test_fprint_fp_dec_helper( __moss_infd(), -1, false );
	//test_fprint_fp_dec_helper( __moss_nand(), -1, false );
	//puts("");

	/*test_fprint_fp_dec_helper( 0.0, -1, false );
	test_fprint_fp_dec_helper( 0.0,  0, false );
	test_fprint_fp_dec_helper( 0.0,  0, true  );
	test_fprint_fp_dec_helper( 0.0,  5, false );
	test_fprint_fp_dec_helper( 0.0, 13, false );
	test_fprint_fp_dec_helper( 0.0, 15, false );
	puts("");

	test_fprint_fp_dec_helper( 3.141592653589793, -1, true  );
	test_fprint_fp_dec_helper( 3.141592653589793,  0, true  );
	test_fprint_fp_dec_helper( 3.141592653589793,  3, true  );
	test_fprint_fp_dec_helper( 3.141592653589793,  5, true  );
	test_fprint_fp_dec_helper( 3.141592653589793, 13, true  );
	test_fprint_fp_dec_helper( 3.141592653589793, 15, true  );
	puts("");

	test_fprint_fp_dec_helper( 12.999999, -1, false );
	test_fprint_fp_dec_helper( 99.999999, -1, false );
	test_fprint_fp_dec_helper( 12.999999,  5, false );
	test_fprint_fp_dec_helper( 99.999999,  5, false );
	puts("");

	test_fprint_fp_dec_helper(    0.0              , -1, false );
	test_fprint_fp_dec_helper( DBL_TRUE_MIN        , -1, false );
	test_fprint_fp_dec_helper(    0.00216          , -1, false );
	test_fprint_fp_dec_helper(    1.0              , -1, false );
	test_fprint_fp_dec_helper(    3.141592653589793, -1, false );
	test_fprint_fp_dec_helper( 1234.0              , -1, false );*/
	test_fprint_fp_dec_helper( DBL_MAX             , -1, false );
}

static void test_fprint_fp_hex_helper( double val, char specifier, int prec, bool always_pt )
{
	char buf[256];
	FILE file_str = __moss_file_str( buf, 256 );

	int n = __moss_fprint_fp_hex( &file_str, val, specifier, prec, always_pt );

	printf( "(fmt=%c,prec=%+3d,sep=%d)   %%f   ->   \"%s\" (%d,%zu)\n",
		specifier,prec,(int)always_pt, /*val,*/ buf, n,strlen(buf)
	);
}
static void test_fprint_fp_hex(void)
{
	test_fprint_fp_hex_helper( __moss_infd(), 'a', -1, false );
	test_fprint_fp_hex_helper( __moss_nand(), 'a', -1, false );
	puts("");

	test_fprint_fp_hex_helper( 0.0, 'a', -1, false );
	test_fprint_fp_hex_helper( 0.0, 'a',  0, false );
	test_fprint_fp_hex_helper( 0.0, 'a',  0, true  );
	test_fprint_fp_hex_helper( 0.0, 'a',  5, false );
	test_fprint_fp_hex_helper( 0.0, 'a', 13, false );
	test_fprint_fp_hex_helper( 0.0, 'a', 15, false );
	puts("");

	test_fprint_fp_hex_helper( 3.141592653589793, 'a', -1, true  );
	test_fprint_fp_hex_helper( 3.141592653589793, 'a',  0, true  );
	test_fprint_fp_hex_helper( 3.141592653589793, 'a',  3, true  );
	test_fprint_fp_hex_helper( 3.141592653589793, 'a',  5, true  );
	test_fprint_fp_hex_helper( 3.141592653589793, 'a', 13, true  );
	test_fprint_fp_hex_helper( 3.141592653589793, 'a', 15, true  );
}





/*int main()
{
	fputs( "Hello world!\n", stdout );
	(void)getchar();
	return ch;
}*/



//From https://en.cppreference.com/w/cpp/io/c/fprintf

//#include <cinttypes>
//#include <cstdint>
//#include <cstdio>
//#include <limits>

void __moss_ftoa( char zeroed_buffer[309+1], double val, char specifier,int prec );

static void test( double val, char specifier/*='g'*/ )
{
	char zeroed_buffer[309+1];
	for ( int i=0; i<310; ++i ) zeroed_buffer[i]='\0';

	__moss_ftoa( zeroed_buffer, val, specifier,-1 );

	printf("\"%s\"\n",zeroed_buffer);
}

static void test_sprint_sz( uint64_t sz, bool use_bin )
{
	char buf[10];
	__moss_sprint_sz_4sigfigs( sz, buf, use_bin );
	printf( "%" PRIu64 " -> \"%s\" (%zu)\n", sz, buf,strlen(buf) );
}

int main( int /*arc*/, char* /*argv*/[] )
{
	//test_bcd();
	//test_fprint_dec_u();
	test_fprint_fp_dec();
	//test_fprint_fp_hex();

	/*test( 0.0 );
	test( 0.0, 'e' );

	test( 3.14159265 );
	test( 2345876234987652938746283746234.0 );
	test( 1.0/3.0 );
	test( numeric_limits<double>::quiet_NaN() );
	test( -numeric_limits<double>::infinity() );*/

	//printf( "Unicode string: \"%ls\"\n", L"this is terrible" );

	/*test_sprint_sz(  68719476736ull, false );
	test_sprint_sz(  99999999999ull, false );
	test_sprint_sz( 999999999999ull, false );
	test_sprint_sz(         1000ull, false );
	test_sprint_sz(      1048576ull, true  );
	test_sprint_sz(     87254999ull, false );*/

	// Sizes of address map from BOOTBOOT in 4096 MB RAM VirtualBox
	//test_sprint_sz(     655360ull, true );
	//test_sprint_sz( 3716042752ull, true );
	//test_sprint_sz(    1249280ull, true );
	//test_sprint_sz(      24576ull, true );
	//test_sprint_sz(    7593984ull, true );
	//test_sprint_sz(     299008ull, true );
	//test_sprint_sz(   11464704ull, true );
	//test_sprint_sz(    2621440ull, true );
	//test_sprint_sz(     589824ull, true );
	//test_sprint_sz(    3588096ull, true );
	//test_sprint_sz(   13127680ull, true );
	//test_sprint_sz(    4194304ull, true );
	//test_sprint_sz(  536870912ull, true );

	#ifdef __MOSS_CLANG__
		#pragma clang diagnostic push
		#pragma clang diagnostic ignored "-Wformat"
		#pragma clang diagnostic ignored "-Wformat-invalid-specifier"
		#pragma clang diagnostic ignored "-Wformat-extra-args"
	#endif
	//printf( "Variant pointer: \"%#p\"\n", NULL );
	//printf( "Recursive format string: %v back to normal %d\n", "(is here making an int: %d)", 6, 4 );
	#ifdef __MOSS_CLANG__
		#pragma clang diagnostic pop
	#endif

	#if 0
	#if 1
	char const* s = "Hello";
	printf( "Strings:\n" );                                // "Strings:"
	printf( "\t[%10s]\n", s );                             // "	[     Hello]"
	printf( "\t[%-10s]\n", s );                            // "	[Hello     ]"
	printf( "\t[%*s]\n", 10, s );                          // "	[     Hello]"
	printf( "\t[%-10.*s]\n", 4, s );                       // "	[Hell      ]"
	printf( "\t[%-*.*s]\n", 10, 4, s );                    // "	[Hell      ]"
	printf( "Characters:\t%c %%\n", 'A' );                 // "Characters:	A %"
	printf("Integers:\n");                                 // "Integers:"
	printf( "\tDecimal:    \t%i %d %.6i %i %.0i %+i %i\n", // "	Decimal:    	1 2 000003 0  +4 -4"
	                          1, 2,   3, 0,   0,  4,-4
	);
	printf( "\tHexadecimal:\t%x %x %X %#x\n",              // "	Hexadecimal:	5 a A 0x6"
	                          5, 10,10, 6
	);
	printf( "\tOctal:      \t%o %#o %#o\n",                // "	Octal:      	12 012 04"
	                          10, 10, 4
	);

	printf("Floating point:\n");                                            // "Floating point:"
	#endif
	//TODO: wrong output here
	printf( "\tRounding:\t%f %.0f %.32f\n", 1.5, 1.5, 1.3 );                // "	Rounding:	1.500000 2 1.30000000000000004440892098500626"
	printf( "\tPadding:\t%05.2f %.2f %5.2f\n", 1.5, 1.5, 1.5 );             // "	Padding:	01.50 1.50  1.50"
	printf( "\tScientific:\t%E %e\n", 1.5, 1.5 );                           // "	Scientific:	1.500000E+00 1.500000e+00"
	printf( "\tHexadecimal:\t%a %A\n", 1.5, 1.5 );                          // "	Hexadecimal:	0x1.8p+0 0X1.8P+0"
	printf( "\tSpecial values:\t0/0=%g 1/0=%g\n", 0.0/0.0, 1.0/0.0 );       // "	Special values:	0/0=-nan 1/0=inf"

	printf("Variable width control:\n");                                    // "Variable width control:"
	        printf( "\tright-justified variable width: '%*c'\n",  5, 'x' ); // "	right-justified variable width: '    x'"
	int r = printf( "\tleft-justified variable width : '%*c'\n", -5, 'x' ); // "	left-justified variable width : 'x    '"
	printf( "(the last printf printed %d characters)\n", r );               // "(the last printf printed 41 characters)"

	printf("Fixed-width types:\n");                                         // "Fixed-width types:"
	//uint32_t val = numeric_limits<uint32_t>::max();
	//printf("\tLargest 32-bit value is %" PRIu32 " or %#" PRIx32 "\n",       // "	Largest 32-bit value is 4294967295 or 0xffffffff"
	//                                           val,            val
	//);
	#endif

	(void)getchar();

	return 0;
}
