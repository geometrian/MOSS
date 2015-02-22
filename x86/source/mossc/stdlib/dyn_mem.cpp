#include "dyn_mem.h"

#include "../../kernel/memory/simple.h"
#include "../../kernel/kernel.h"
#include "../string/other.h"


void* calloc(size_t num, size_t size) {
	void* result = malloc(num*size);
	memset(result,0,size);
	return result;
}
void* malloc(size_t size) {
	#ifdef MOSS_DEBUG
	ASSERT(MOSS::Kernel::memory!=NULL,"Cannot allocate memory; memory manager does not exist!");
	#endif
	return MOSS::Kernel::memory->malloc(size);
}
void* realloc(void* ptr, size_t size) {
	free(ptr);
	if (size==0u) {
		return NULL;
	}
	return malloc(size);
}
void free(void* ptr) {
	#ifdef MOSS_DEBUG
	ASSERT(MOSS::Kernel::memory!=NULL,"Cannot delete memory; memory manager does not exist!");
	#endif
	MOSS::Kernel::memory->free(ptr);
}

void* operator new  (size_t size) {
	return malloc(size);
}
void* operator new[](size_t size) {
	return malloc(size);
}
void operator delete  (void* p) {
	free(p);
}
void operator delete[](void* p) {
	free(p);
}