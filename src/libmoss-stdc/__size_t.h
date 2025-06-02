#pragma once

#include "__env.h"

//#if MOSS_ENV == MOSS_ENV_EMBEDWIN
#ifdef MOSS_USE_WRONG_SIZE_T

	#if   defined __MOSS_32__
		typedef unsigned int size_t;
	#elif defined __MOSS_64__
		typedef unsigned long long size_t;
	#else
		#error
	#endif

#else

	/*
	Define it like this since the compiler has ideas about what it needs to be.  E.g. with
	"--target=x86_64-unknown-none-elf", it is 4, even on 64-bit, but on e.g.
	"--target x86_64-pc-win32-coff" it is 8 for 64-bit.  Anyway, it's supposed to be the type of the
	`sizeof` so this is more accurate anyway.
	*/
	typedef __typeof__(sizeof(0)) size_t;

#endif

/* TODO: assert size */
