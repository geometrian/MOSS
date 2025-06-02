#ifdef __INTELLISENSE__
	#undef __cplusplus
#endif

#include "stdio.h"

#include <inttypes.h>

#include "__env.h"
#include "assert.h"
#include "math.h"
#include "stdint.h"
#include "string.h"



__MOSS_FILE __moss_stdin  = { MOSS_STREAM_STDIN , .fn={NULL,NULL} };
__MOSS_FILE __moss_stdout = { MOSS_STREAM_STDOUT, .fn={NULL,NULL} };
__MOSS_FILE __moss_stderr = { MOSS_STREAM_STDERR, .fn={NULL,NULL} };

__MOSS_FILE __moss_file_fn( int(*fn)(int ch,void* data), void* data )
{
	return (FILE){ MOSS_STREAM_FUNCTION, .fn={fn,data} };
}
__MOSS_FILE __moss_file_str( char buf[], size_t size )
{
	__moss_assert( size>0, "Buffer cannot be empty!" );
	buf[0] = '\0';
	return (FILE){ MOSS_STREAM_STRING, .str={buf,size} };
}



/* for `vfprintf(⋯)`, see "stdio_vfprintf.c" */
MOSS_VARFN_ATTRIB( printf, 1, 0 )
int   vprintf(                           char const* fmt_cstr,va_list args )
{
	return vfprintf( stdout, fmt_cstr,args );
}
MOSS_VARFN_ATTRIB( printf, 3, 0 )
int vsnprintf( char* buf,size_t bufsize, char const* fmt_cstr,va_list args )
{
	FILE stream = __moss_file_str( buf, bufsize );
	int ret = vfprintf( &stream, fmt_cstr,args );
	if ( ret >= 0 )
	{
		int ret2 = fputc( '\0', &stream ); //Note not counted in `ret`
		if ( ret2 < 0 ) return ret2;
	}
	return ret;
}
MOSS_VARFN_ATTRIB( printf, 2, 0 )
int  vsprintf( char* buf,                char const* fmt_cstr,va_list args )
{
	FILE stream = __moss_file_str( buf, ~(size_t)0 );
	int ret = vfprintf( &stream, fmt_cstr,args );
	if ( ret >= 0 )
	{
		int ret2 = fputc( '\0', &stream ); //Note not counted in `ret`
		if ( ret2 < 0 ) return ret2;
	}
	return ret;
}

MOSS_VARFN_ATTRIB( printf, 2, 3 )
int  fprintf( FILE* stream,              char const* fmt_cstr,...         )
{
	va_list args;
	va_start( args, fmt_cstr );

	int ret = vfprintf( stream, fmt_cstr,args );

	va_end( args );

	return ret;
}
MOSS_VARFN_ATTRIB( printf, 1, 2 )
int   printf(                            char const* fmt_cstr,...         )
{
	va_list args;
	va_start( args, fmt_cstr );

	int ret = vprintf( fmt_cstr,args );

	va_end( args );

	return ret;
}
MOSS_VARFN_ATTRIB( printf, 3, 4 )
int snprintf( char* buf,size_t bufsize,  char const* fmt_cstr,...         )
{
	va_list args;
	va_start( args, fmt_cstr );

	int ret = vsnprintf( buf,bufsize, fmt_cstr,args );

	va_end( args );

	return ret;
}
MOSS_VARFN_ATTRIB( printf, 2, 3 )
int  sprintf( char* buf,                 char const* fmt_cstr,...         )
{
	va_list args;
	va_start( args, fmt_cstr );

	int ret = vsprintf( buf, fmt_cstr,args );

	va_end( args );

	return ret;
}

int __moss_sprint_sz_4sigfigs( uint64_t sz, char buf[10], _Bool use_bin )
{
	if ( sz <= 9999 )
	{
		return sprintf( buf, "%" PRIu64 " B", sz );
	}

	// Biggest is 2⁶⁴-1 = 18,446,744,073,709,551,615 ≈ 18.46 ZB
	char const* power_name = __moss_si_pos;
	uint16_t factor = use_bin ? 1024 : 1000;

	uint64_t div = factor;
	LOOP:
	{
		uint64_t div_next = div * factor;
		if ( div_next <= sz )
		{
			div = div_next;
			++power_name;
			goto LOOP;
		}
	}

	// TODO: integer arithmetic
	double coeff = (double)sz / (double)div;
	__moss_assert_impl( coeff>=1.0 && coeff<(double)factor );
	if      ( coeff >= 100.0 )
	{
		coeff = round( coeff *   10.0 );
		if ( coeff >= 10000.0 ) { coeff/=(double)factor; ++power_name; }
		coeff *= 0.1;
	}
	else if ( coeff >=  10.0 )
	{
		coeff = round( coeff * 100.0 );
		coeff *= 0.01;
	}
	else
	{
		coeff = round( coeff * 1000.0 );
		coeff *= 0.001;
	}
	__moss_assert_impl( coeff>=0.0 && coeff<(double)factor );

	int n;
	if      ( coeff >= 100.0 ) n=sprintf( buf, "%.1f", coeff );
	else if ( coeff >=  10.0 ) n=sprintf( buf, "%.2f", coeff );
	else                       n=sprintf( buf, "%.3f", coeff );
	__moss_assert( n==5, "Failed to convert %f (from  %" PRIu64 ") to string!", coeff, sz );

	char* dst = buf + n;
	*(dst++) = ' ';
	*(dst++) = *power_name;
	if ( use_bin ) *(dst++)='i';
	*(dst++) = 'B';
	n = (int)( dst - buf );
	__moss_assert_impl( n <= 10 );

	*(dst++) = '\0';

	return n;
}

/*
TODO

MOSS_VARFN_ATTRIB( scanf, 2, 0 )
int vfscanf( FILE* stream,    char const* fmt_cstr,va_list args ) { ... }
MOSS_VARFN_ATTRIB( scanf, 1, 0 )
int  vscanf(                  char const* fmt_cstr,va_list args ) { ... }
MOSS_VARFN_ATTRIB( scanf, 2, 0 )
int vsscanf( char const* str, char const* fmt_cstr,va_list args ) { ... }

MOSS_VARFN_ATTRIB( scanf, 2, 3 )
int  fscanf( FILE* stream,    char const* fmt_cstr,...         )
{
	va_list args;
	va_start( args, fmt_cstr );

	int ret = vfscanf( stream, fmt_cstr,args );

	va_end( args );

	return ret;
}
MOSS_VARFN_ATTRIB( scanf, 1, 2 )
int   scanf(                  char const* fmt_cstr,...         )
{
	va_list args;
	va_start( args, fmt_cstr );

	int ret = vscanf( fmt_cstr,args );

	va_end( args );

	return ret;
}
MOSS_VARFN_ATTRIB( scanf, 2, 3 )
int  sscanf( char const* str, char const* fmt_cstr,...         )
{
	va_list args;
	va_start( args, fmt_cstr );

	int ret = vsscanf( str, fmt_cstr,args );

	va_end( args );

	return ret;
}
*/

int fputc         ( int ch, FILE* stream            )
{
	switch ( stream->type )
	{
		case MOSS_STREAM_STDIN :
			return EOF;
		case MOSS_STREAM_STDOUT:
			return __moss_putc_stdout(ch);
		case MOSS_STREAM_STDERR:
			return __moss_putc_stderr(ch);
		case MOSS_STREAM_FUNCTION:
		{
			return stream->fn.ptr( ch, stream->fn.user_data );
		}
		case MOSS_STREAM_STRING:
		{
			if ( stream->str.remaining_sz <= 1 ) return EOF;

			char* dst = (char*)stream->str.buf;
			dst[0] = (char)ch;
			dst[1] = '\0';
			++stream->str.buf;

			if ( stream->str.remaining_sz != ~((size_t)0) ) --stream->str.remaining_sz;

			return ch;
		}
		__moss_switch_covered_enum;
	}
}
int __moss_fputc_n( int ch, FILE* stream, int count )
{
	int num_wrote;
	for ( num_wrote=0; num_wrote<count; ++num_wrote )
	{
		if ( fputc(ch,stream) == EOF ) return EOF;
	}
	return num_wrote;
}
int fputs( char const* str, FILE* stream )
{
	char const* str_orig = str;
	LOOP:
		if ( *str != '\0' )
		{
			if ( fputc(*str,stream) != EOF )
			{
				++str;
				goto LOOP;
			}
			else return EOF;
		}
	return (int)( str - str_orig );
}
void __moss_fput_int( intmax_t    val, FILE* stream )
{
	// Longest output is "-2147483648" (11 characters, not null-terminated)
	static_assert( sizeof(int) <= 4 );
	char buf[ 11 ];

	// Write integer backward
	char* dst = buf;

	if ( val != 0 )
	{
		if ( val == INTMAX_MIN )
		{
			/*
			We need a bit of help because we can't just negate it.  Output the lowest
			digit first.  Value is of the form -2ʰ⁻¹, where h∈{8,16,32,64}.  So, the
			last digit is an '8':
				-128 -32768, -2147483648
			*/
			*(dst++) = '8';
			val = ( val + 8 ) / 10;
		}

		int is_neg = 0;
		if ( val < 0 ) { val=-val; is_neg=1; }

		while ( val > 0 )
		{
			*(dst++) = '0' + val%10;
			val /= 10;
		}

		if ( is_neg ) *(dst++)='-';
	}
	else *(dst++)='0';

	// Write backward buffer backward, thus writing it forward
	--dst;
	while ( dst >= buf )
	{
		fputc( *dst, stream );
		--dst;
	}
}
void __moss_fput_ptr( void const* val, FILE* stream )
{
	union { void const* ptr; uintptr_t uint; } u;
	u.ptr = val;

	for ( int nyb=sizeof(uintptr_t)*2-1; nyb>=0; --nyb )
	{
		uintptr_t tmp = ( u.uint >> 4*nyb ) & 0xF;
		char ch = __moss_xchrs[ tmp ];
		fputc( ch, stream );
	}
}
#ifndef _MSC_VER
void __moss_fput_stack( FILE* stream )
{
	/*
	A typical stack (current routine 2, called by preceding routine 1) looks like this:

		                      call  │                                         ^
		                      info ─┤   ┃  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  ┃   |
		0x00104FF8 ━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫ <━━━ Routine 0 ESP
		               ROUTINE 1    │   ┃ Return to routine 0 EBP value   ╂ ─ ┘
		0x00104FF4    STACK FRAME   └─> ┠─────────────────────────────────┨ <━┯━ Routine 1 EBP
		                                ┃ Local variable                  ┃   |
		0x00104FF0                  ┌─> ┠─────────────────────────────────┨   |
		                      call ─┤   ┃ Routine 2 argument 2            ┃   |
		0x00104FEC            info  │   ┠ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┨   |
		                            │   ┃ Routine 2 argument 1            ┃   |
		0x00104FE8                  │   ┠ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┨   |
		                            │   ┃ Return to routine 1 EIP address ┃   |
		0x00104FE4 ━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫ <━━━ Routine 1 ESP
		               ROUTINE 2    │   ┃ Return to routine 1 EBP value   ╂ ─ ┘
		0x00104FE0    STACK FRAME   └─> ┠─────────────────────────────────┨ <━━━ Routine 2 EBP
		                                ┃ Local variable                  ┃      (Current EBP)
		0x00104FDC                      ┠ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┨
		                                ┃ Local variable                  ┃
		0x00104FD8                      ┠ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┨
		                                ┃ Local variable                  ┃
		0x00104FD4 ━━━━━━━━━━━━━━━━━━━━━┻━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛ <━━━ Routine 2 ESP
		                                                                         (Current ESP)

	The arguments have been pushed in the caller stack frame, then there's the return address the
	`call` instruction pushes before jumping to routine 2.  Then there's the caller's EBP value,
	saved by the callee so we can return.  The EBP in each frame is the beginning of the local
	variables and the ESP in each frame is the end of the stack frame.  See "doc/" for a fuller
	description.

	The basic format of the output is:

		"+-------+------------------+------------------+------------+------------------+------------------+\n"
		"| Frame | Frame Pointer    | Stack Pointer    | (size [B]) | Return Frame     | Return Frame     |\n"
		"| Depth | (RBP)            | (RSP)            |            | Pointer (RBP)    | Address (RIP)    |\n"
		"+-------+------------------+------------------+------------+------------------+------------------+\n"
		"| ##### | ffffffffffffffff | ffffffffffffffff | ########## | ffffffffffffffff | ffffffffffffffff |\n"
		"+-------+------------------+------------------+------------+------------------+------------------+\n"
	*/

	// First thing is to grab the current frame pointer (EBP) and stack pointer (ESP)
	void const* frame_ptr;
	void const* stack_ptr;
	if ( sizeof(void const*) == 4 )
	{
		asm(
			"mov   %0, esp\n"
			"mov   %1, ebp\n"
			: "=rm"(stack_ptr), "=rm"(frame_ptr) : :
		);
	}
	else
	{
		asm(
			"mov   %0, rsp\n"
			"mov   %1, rbp\n"
			: "=rm"(stack_ptr), "=rm"(frame_ptr) : :
		);
	}

	// Note not using `fprintf(⋯)` because it has a big stack footprint, and if we're calling this,
	// we're probably debugging the stack.
	fputs(
		"+-------+------------------+------------------+------------+------------------+------------------+\n"
		"| Frame | Frame Pointer    | Stack Pointer    | (size [B]) | Return Frame     | Return Frame     |\n"
		"| Depth | (",
		stream
	);
	fputs( sizeof(void const*)==4?"EBP":"RBP", stream );
	fputs(
		              ")            | (",
		stream
	);
	fputs( sizeof(void const*)==4?"ESP":"RSP", stream );
	fputs(
		                                 ")            |            | Pointer (",
		stream
	);
	fputs( sizeof(void const*)==4?"EBP":"RBP", stream );
	fputs(
		                                                                         ")    | Address (",
		stream
	);
	fputs( sizeof(void const*)==4?"EIP":"RIP", stream );
	fputs(
		                                                                                            ")    |\n"
		"+-------+------------------+------------------+------------+------------------+------------------+\n",
		stream
	);

	// Now we need to get the return information, which straddles the stack frame boundary.  The
	// caller's callee-saved EBP is the first thing on the stack frame, and return address is the
	// last thing in the previous stack frame.  Remember that the stack grows downward, so they are
	// in that order in memory.
	struct ReturnInfo
	{
		void const* ret_frame_ptr;
		void const* ret_addr;
	};
	struct ReturnInfo ret_info;
	ret_info = *(struct ReturnInfo const*)frame_ptr;

	// Don't walk the stack here to e.g. count the stack frames, because the stack might be
	// corrupted and doing so would segfault.  We need to at least print something first.

	// Walk up the stack, outputting as we go
	int depth = 0;
	while (1)
	{
		fputs("| ",stream);

		if      ( depth ==    0 ) fputs("    ",stream);
		else if ( depth >=   -9 ) fputs("   " ,stream);
		else if ( depth >=  -99 ) fputs("  "  ,stream);
		else if ( depth >= -999 ) fputs(" "   ,stream);
		__moss_fput_int( depth--, stream );

		fputs(" | ",stream);

		__moss_fput_ptr( frame_ptr, stream );

		fputs(" | ",stream);

		__moss_fput_ptr( stack_ptr, stream );

		fputs(" | ",stream);

		int size = (int)( (uint8_t const*)frame_ptr - (uint8_t const*)stack_ptr );
		if      ( size >= 100000000 ) fputs(" "        ,stream);
		else if ( size >=  10000000 ) fputs("  "       ,stream);
		else if ( size >=   1000000 ) fputs("   "      ,stream);
		else if ( size >=    100000 ) fputs("    "     ,stream);
		else if ( size >=     10000 ) fputs("     "    ,stream);
		else if ( size >=      1000 ) fputs("      "   ,stream);
		else if ( size >=       100 ) fputs("       "  ,stream);
		else if ( size >=        10 ) fputs("        " ,stream);
		else                          fputs("         ",stream);
		__moss_fput_int( size, stream );

		fputs(" | ",stream);

		__moss_fput_ptr( ret_info.ret_frame_ptr, stream );

		fputs(" | ",stream);

		__moss_fput_ptr( ret_info.ret_addr, stream );

		fputs(" |\n",stream);

		if ( ret_info.ret_frame_ptr == 0 ) break;

		stack_ptr = (void const*)( (uintptr_t const*)frame_ptr + 1 );
		frame_ptr = ret_info.ret_frame_ptr;
		ret_info = *(struct ReturnInfo const*)frame_ptr;
	}

	fputs(
		"+-------+------------------+------------------+------------+------------------+------------------+\n",
		stream
	);
}
#endif

int getc( FILE* stream )
{
	switch ( stream->type )
	{
		case MOSS_STREAM_STDIN   :
			return __moss_getc_stdin();
		case MOSS_STREAM_STDOUT  : return EOF;
		case MOSS_STREAM_STDERR  : return EOF;
		case MOSS_STREAM_FUNCTION: return EOF;
		case MOSS_STREAM_STRING  : return EOF;
		__moss_switch_covered_enum;
	}
}
int getch( void )
{
	return __moss_getch();
}
int getchar( void )
{
	return getc( stdin );
}

int puts( char const* str )
{
	int count=0, ret;

	while ( *str != '\0' )
	{
		ret = putchar( *str );
		if ( ret == EOF ) return ret;
		++count;

		++str;
	}

	ret = putchar( '\n' );
	if ( ret == EOF ) return ret;
	++count;

	return count;
}
