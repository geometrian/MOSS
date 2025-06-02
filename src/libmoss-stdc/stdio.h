#pragma once

/*
https://en.cppreference.com/w/cpp/header/cstdio
*/



/* #include "__arch.h" */
#include "__size_t.h"
#include <stdarg.h>
#include <stdint.h>



#ifdef __MOSS_CPP__
extern "C"
{
#endif

typedef enum
{
	MOSS_STREAM_STDIN    = 0,
	MOSS_STREAM_STDOUT   = 1,
	MOSS_STREAM_STDERR   = 2,

	MOSS_STREAM_FUNCTION = 3, /* see below for an example of this */
	MOSS_STREAM_STRING   = 4  /* and this */
}
__MOSS_FILE_TYPE;

typedef struct { int(*ptr)(int,void*); void* user_data; } __MOSS_FILE_Fn ;
typedef struct { char* buf; size_t remaining_sz;        } __MOSS_FILE_Str;
typedef struct
{
	__MOSS_FILE_TYPE type;
	union { __MOSS_FILE_Str str; __MOSS_FILE_Fn fn; };
}
__MOSS_FILE;

extern __MOSS_FILE __moss_stdin ;
extern __MOSS_FILE __moss_stdout;
extern __MOSS_FILE __moss_stderr;

/*
int my_write( int ch, void* user_data ) { <do something> ; return okay?ch:EOF; }
FILE file_fn = __moss_file_fn( my_write, some_user_data );

char buf[ 64 ];
FILE file_str = __moss_file_str( buf, 64 );
*/
__MOSS_FILE __moss_file_fn( int(*fn)(int ch,void* data), void* data );
__MOSS_FILE __moss_file_str( char buf[], size_t size );

#ifdef __MOSS_CPP__
}
#endif



#ifdef __MOSS_CPP__
namespace std
{
	extern "C"
	{
#endif



typedef __MOSS_FILE FILE;

typedef unsigned long long fpos_t;



#ifndef NULL
	#define NULL 0
#endif

/*
TODO

#define _IOFBF
#define _IOLBF
#define _IONBF
#define BUFSIZ
*/

#define EOF ((int)(-1))

/*
TODO

#define FOPEN_MAX
#define FILENAME_MAX
#define L_tmpnam

#define SEEK_CUR
#define SEEK_END
#define SEEK_SET

#define TMP_MAX
*/

#define stdin  (&__moss_stdin )
#define stdout (&__moss_stdout)
#define stderr (&__moss_stderr)


/*
TODO

int remove( char const* path );
int rename( char const* path_old, char const* path_new );

FILE* tmpfile( void );
char* tmpnam( char* path_buffer_opt );

int fclose( FILE* stream );
int fflush( FILE* stream );

FILE* fopen  ( char const* file_path, char const* mode               );
FILE* freopen( char const* file_path, char const* mode, FILE* stream );

void setbuf ( FILE* stream, char* buf                        );
int  setvbuf( FILE* stream, char* buf, int mode, size_t size );
*/

MOSS_VARFN_ATTRIB( printf, 2, 0 )
int  vfprintf( FILE* stream,             char const* fmt_cstr,va_list args );
MOSS_VARFN_ATTRIB( printf, 1, 0 )
int   vprintf(                           char const* fmt_cstr,va_list args );
MOSS_VARFN_ATTRIB( printf, 3, 0 )
int vsnprintf( char* buf,size_t bufsize, char const* fmt_cstr,va_list args );
MOSS_VARFN_ATTRIB( printf, 2, 0 )
int  vsprintf( char* buf,                char const* fmt_cstr,va_list args );

MOSS_VARFN_ATTRIB( printf, 2, 3 )
int  fprintf( FILE* stream,              char const* fmt_cstr,...          );
MOSS_VARFN_ATTRIB( printf, 1, 2 )
int   printf(                            char const* fmt_cstr,...          );
MOSS_VARFN_ATTRIB( printf, 3, 4 )
int snprintf( char* buf,size_t bufsize,  char const* fmt_cstr,...          );
MOSS_VARFN_ATTRIB( printf, 2, 3 )
int  sprintf( char* buf,                 char const* fmt_cstr,...          );

/* E.g. "1024 B", "65.54 kB", "1.000 GiB" */
int __moss_sprint_sz_4sigfigs( uint64_t sz, char buf[10], _Bool use_bin );

/*
TODO

MOSS_VARFN_ATTRIB( scanf, 2, 0 )
int vfscanf( FILE* stream,    char const* fmt_cstr,va_list args );
MOSS_VARFN_ATTRIB( scanf, 1, 0 )
int  vscanf(                  char const* fmt_cstr,va_list args );
MOSS_VARFN_ATTRIB( scanf, 2, 0 )
int vsscanf( char const* str, char const* fmt_cstr,va_list args );

MOSS_VARFN_ATTRIB( scanf, 2, 3 )
int  fscanf( FILE* stream,    char const* fmt_cstr,...          );
MOSS_VARFN_ATTRIB( scanf, 1, 2 )
int   scanf(                  char const* fmt_cstr,...          );
MOSS_VARFN_ATTRIB( scanf, 2, 3 )
int  sscanf( char const* str, char const* fmt_cstr,...          );

int fgetc(FILE* stream);
char* fgets(char* s, int n, FILE* stream);
*/

/*
Writes `c` to `stream` `count` times.  If write fails, return `EOF`, else the number written (i.e.
`count`).
*/

int fputc         ( int ch, FILE* stream            );
int __moss_fputc_n( int ch, FILE* stream, int count );
int fputs( char const* str, FILE* stream );
void __moss_fput_int( intmax_t    val, FILE* stream );
void __moss_fput_ptr( void const* val, FILE* stream );
#ifndef _MSC_VER
void __moss_fput_stack( FILE* stream );
#endif

/* get from a file */
int getc( FILE* stream );
/* get from `stdin` without waiting for a newline */
int getch( void );
/* get from `stdin`, waiting for a newline */
int getchar( void );

MOSS_INLINE int putc( int ch, FILE* stream ) { return fputc(ch,stream); }
MOSS_INLINE int putchar( int ch ) { return putc(ch,stdout); }
int puts( char const* str );

/*
int ungetc(int c, FILE* stream);
size_t fread(void* ptr, size_t size, size_t nmemb, FILE* stream);
size_t fwrite(void const* ptr, size_t size, size_t nmemb, FILE* stream);
int fgetpos(FILE* stream, fpos_t* pos);
int fseek(FILE* stream, long int offset, int whence);
int fsetpos(FILE* stream, fpos_t const* pos);
long int ftell(FILE* stream);
void rewind(FILE* stream);
void clearerr(FILE* stream);
int feof(FILE* stream);
int ferror(FILE* stream);
void perror(char const* s);*/



#ifdef __MOSS_CPP__
	}
}
#endif
