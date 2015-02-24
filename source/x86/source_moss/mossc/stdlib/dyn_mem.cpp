#include "dyn_mem.h"

#include "../../kernel/memory/simple.h"
#include "../../kernel/kernel.h"

#include "../string/other.h"


namespace MOSSC {


void* calloc(size_t num, size_t size) {
	void* result = malloc(num*size);
	memset(result, 0, size);
	return result;
}
void* malloc(size_t size) {
	assert_term(MOSS::kernel->memory!=nullptr,"Cannot allocate memory; memory manager does not exist!");
	return MOSS::kernel->memory->malloc(size);
}
void* realloc(void* ptr, size_t size) {
	free(ptr);
	if (size==0) return nullptr;
	return malloc(size);
}
void free(void* ptr) {
	assert_term(MOSS::kernel->memory!=nullptr,"Cannot delete memory; memory manager does not exist!");
	MOSS::kernel->memory->free(ptr);
}


}


void* operator new  (size_t size) {
	return MOSSC::malloc(size);
}
void* operator new[](size_t size) {
	return MOSSC::malloc(size);
}
void operator delete  (void* p) {
	MOSSC::free(p);
}
void operator delete[](void* p) {
	MOSSC::free(p);
}
