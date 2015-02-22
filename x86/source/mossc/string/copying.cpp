#include "copying.h"


namespace MOSSC {


#if 1
void* memcpy(void* destination, const void* source, size_t num) {
	char* dst = (char*)(destination);
	const char* src = (const char*)(source);

	for (size_t i=0u;i<num;++i) {
		dst[i] = src[i];
	}

	return destination;
}
#else
/*void* memcpy(void* destination, const void* source, size_t num) {
	char* dst = (char*)(destination);
	const char* src = (const char*)(source);

	//Also handles alignment for 32-bit!  Count down because testing against zero is fastest.
	int num_begin = 4 - (int)(dst)%4;
	if (num<(size_t)(num_begin)) {

	}
}*/
//Adapted from http://forum.osdev.org/viewtopic.php?t=18119
void* memcpy(void* destination, const void* source, size_t num) {
	//TODO: also support SSE/MMX!
	if (cpu.feature.flags.edx.sse) {
		int i;
		for(i=0; i<n/16; i++)
		{
			__asm__ __volatile__ ("movups (%0), %%xmm0\n" "movntdq %%xmm0, (%1)\n"::"r"(from), "r"(to) : "memory");

			from += 16;
			to += 16;
		}
	}
	else if (n&15 && cpu.feature.flags.edx.mmx) {
		n = n&15;
		int i;
		for(i=0; i<n/8; i++)
		{
			__asm__ __volatile__ ("movq (%0), %%mm0\n" "movq %%mm0, (%1)\n"::"r"(from), "r"(to):"memory");
			from += 8;
			to += 8;
		}
	}
	if (n & 7) {
		n = n&7;

		int d0, d1, d2;
		__asm__ __volatile__(
		"rep ; movsl\n\t"
		"testb $2,%b4\n\t"
		"je 1f\n\t"
		"movsw\n"
		"1:\ttestb $1,%b4\n\t"
		"je 2f\n\t"
		"movsb\n"
		"2:"
		: "=&c" (d0), "=&D" (d1), "=&S" (d2)
		:"0" (n/4), "q" (n),"1" ((long) to),"2" ((long) from)
		: "memory");
	}
	return destination;
}
#endif
void* memmove(void* destination, const void* source, size_t num) {
	//Difference is that the pointers can overlap

	if (num!=0u) {
		char* dst = (char*)(destination);
		const char* src = (const char*)(source);

		if (dst>src) {
			//Moving forward in memory (so do the copy backwards)
			size_t i = num - 1;

			LOOP:
				dst[i] = src[i];
				if (i!=0u) {
					i--;
					goto LOOP;
				}
		} else {
			//Moving backward in memory (so do the copy forwards)
			size_t i = 0u;
			do {
				dst[i] = src[i];
			} while (++i<num);
		}
	}

	return destination;
}

char* strcpy(char* destination, const char* source) {
	int i = 0;

	LOOP:
		char c = source[i];
		destination[i] = c;
		if (c!='\0') {
			++i;
			goto LOOP;
		}

	return destination;
}
char* strncpy(char* destination, const char* source, size_t num) {
	size_t i = 0u;

	LOOP1:
		if (i>=num) return destination;

		char c = source[i];
		if (c=='\0') {
			goto LOOP2;
		}
		destination[i] = c;

		++i;
		goto LOOP1;

	LOOP2:
	do {
		destination[i] = '\0';
	} while (++i<num);

	return destination;
}


}