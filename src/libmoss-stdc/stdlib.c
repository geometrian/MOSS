#include "stdlib.h"

#include "__env.h"
#include "ctype.h"
#include "stddef.h"
#include "stdint.h"
#include "stdio.h"
#include "string.h"



/* "crti.asm" */
//void _fini( void );

__attribute__((noreturn))
void abort( void )
{
	#if   defined __MOSS_CLANG__ || defined __MOSS_GCC__
		__builtin_trap();
	#elif defined __MOSS_MSVC__
		__debugbreak();
	#endif
	//while (1) {}
}

__attribute__((noreturn))
void exit( int /*status*/ )
{
	#if MOSS_ENV != MOSS_ENV_EMBEDWIN
	//_fini();
	#endif

	abort();
}

#if MOSS_ENV != MOSS_ENV_EMBEDWIN

typedef void (*_MOSS_AtExitFn)(void);

static _MOSS_AtExitFn _atexit_fns[32];
uint8_t _atexit_count = 0;

int atexit( _MOSS_AtExitFn fn )
{
	if ( _atexit_count == 0 )
	{
		memset( _atexit_fns, 0x00, sizeof(_MOSS_AtExitFn) );
	}
	else if ( _atexit_count == 32 ) return -1;

	_atexit_fns[ _atexit_count++ ] = fn;

	return 0;
}

#endif



#define MOSS_MEM_SZ ( 64 * 1024 * 1024 )
//#define MOSS_MEM_SZ (16*24)

#ifdef MOSS_DEBUG
	#if   defined __MOSS_32__
		#define MOSS_BLOCK_SIG_VAL 0xAB70C516 /* "A block sig"" */
	#elif defined __MOSS_64__
		#define MOSS_BLOCK_SIG_VAL 0xAB166E4B70C51664 /* "A bigger block sig, 64" */
	#else
		#error ""
	#endif
#endif



struct _Block
{
	#ifdef MOSS_DEBUG
	size_t sig;
	#endif
	struct _Block* prev;
	struct _Block* next;
	size_t alloced_size;
};
typedef struct _Block Block;

static unsigned char _dyn_mem[ MOSS_MEM_SZ ];



static void _ensure_init( void )
{
	static int _inited = 0;
	if ( _inited ) return;

	memset( _dyn_mem, 0x00, MOSS_MEM_SZ );

	Block* first = (Block*)  _dyn_mem                                ;
	Block* last  = (Block*)( _dyn_mem + MOSS_MEM_SZ - sizeof(Block) );
	first->next = last;
	last->prev = first;
	#ifdef MOSS_DEBUG
	first->sig = MOSS_BLOCK_SIG_VAL;
	last ->sig = MOSS_BLOCK_SIG_VAL;
	#endif

	/* treat the final fencepost block as allocated so that it never coalesces with free space */
	last->alloced_size = 1; //~(size_t)0;

	_inited = 1;
}

void __moss_memory_dump( void )
{
	_ensure_init();

	printf( "Memory dump:\n" );
	Block* block = (Block*)_dyn_mem;
	LOOP:
		printf( "  %p: [ block (", (void*)block );
		if      ( block->alloced_size == 0 ) printf( "free"                      );
		else if ( block->alloced_size == 1 ) printf( " end"                      );
		else                                 printf( "%4zu", block->alloced_size );
		printf( "), prev=%p, next=%p ]\n", (void*)block->prev,(void*)block->next );
		__moss_assert( block->sig==MOSS_BLOCK_SIG_VAL, "Detected heap corruption!" );

		block = block->next;
		if ( block != NULL ) goto LOOP;
}

void* malloc( size_t size )
{
	return aligned_alloc( alignof(max_align_t), size );
}
void* aligned_alloc( size_t align, size_t size )
{
	#ifdef __MOSS_CLANG__
		#pragma clang diagnostic push
		#pragma clang diagnostic ignored "-Wcast-align"
	#endif

	if ( align==0 || size%align!=0 || align>0xFFFFFFFFu ) return NULL;
	if ( align>2 && ((align-1)&align) ) return NULL; /* Not a power of 2 */
	if ( align < alignof(max_align_t) ) align=alignof(max_align_t);
	_ensure_init();

	/* Make it a multiple of `Block` in size */
	size += ( sizeof(Block) - size%sizeof(Block) ) % sizeof(Block);

	Block* block = (Block*)_dyn_mem;
	LOOP:
		/* block represents allocated space */
		__moss_assert( block->sig==MOSS_BLOCK_SIG_VAL, "Detected heap corruption!" );
		if ( block->alloced_size != 0 ) goto NEXT;

		/* block is free; get the available space */
		char* block_data = (char*)( block + 1 );
		size_t free_size = (size_t)( (char*)block->next - block_data );

		/* figure additional space for aligning and storing alignment info */
		union { uintptr_t uint; uint32_t* u32ptr; void* vptr; } u;
		u.vptr = block_data;
		uint32_t offset = (uint32_t)( align - u.uint%align );
		if ( offset < sizeof(uint32_t) ) offset+=align;
		size_t real_size = offset + size;

		/* if block has enough space; allocate it! */
		if ( free_size < real_size ) goto NEXT;
		block->alloced_size = real_size;
		free_size -= real_size;
		u.uint += offset;
		u.u32ptr[-1] = offset;

		/* if another (useful) block fits after this new one, set up a node for that as well. */
		if ( free_size > sizeof(Block) )
		{
			Block* free = (Block*)( block_data + block->alloced_size );
			__moss_assert_impl( free->alloced_size == 0 );

			#ifdef MOSS_DEBUG
			free->sig = MOSS_BLOCK_SIG_VAL;
			#endif
			block->next->prev = free;
			free->next = block->next;
			block->next = free;
			free->prev = block;
		}

		/* return allocated block! */
		return u.vptr;

		/* block is unsuitable; try the next one, if available */
		NEXT:
		block = block->next;
		if ( block->next != NULL ) goto LOOP;

	return NULL;

	#ifdef __MOSS_CLANG__
		#pragma clang diagnostic pop
	#endif
}

void free( void* ptr )
{
	if ( ptr == NULL ) return;
	_ensure_init();

	/* get the actual block we're in */
	union { uintptr_t uint; uint32_t* u32ptr; Block* bptr; void* vptr; } u;
	u.vptr = ptr;
	uint32_t offset = u.u32ptr[-1];
	u.uint -= offset;
	Block* block = u.bptr - 1;
	__moss_assert( block->sig==MOSS_BLOCK_SIG_VAL, "Detected invalid free or heap corruption!" );

	/* clear data for next time (leave the sig and pointers; we'll zero them below) */
	block->alloced_size = 0;
	memset( block+1, 0x00, block->alloced_size );

	/* coalesce with previous, if present and likewise empty */
	if ( block->prev!=NULL && block->prev->alloced_size==0 )
	{
		block->prev->next = block->next;
		block->next->prev = block->prev;

		Block* tmp = block->prev;
		memset( block, 0x00, sizeof(Block) ); /* clear sig and pointers */

		block = tmp;
	}

	/* coalesce with next if it is also free */
	if ( block->next->alloced_size == 0 )
	{
		block = block->next;

		block->prev->next = block->next;
		block->next->prev = block->prev;

		memset( block, 0x00, sizeof(Block) ); /* clear sig and pointers */
	}

	return;
}

/* tree allocator (incomplete) */
#if 0

typedef struct
{
	unsigned exists         :  1;
	unsigned left_is_data   :  1;
	unsigned child_r_offset : 30;
}
_Node;

unsigned char _dyn_mem[ MOSS_MEM_SZ ];

static void* _malloc_node( size_t alloc_size, _Node* node,size_t node_data_size )
{
	if ( node_data_size < alloc_size+2*sizeof(_Node) ) return NULL;

	if ( !node->exists )
	{
		node->exists = 1;
		node->left_is_data = 1;
		node->child_r_offset = sizeof(_Node) + alloc_size;
		__moss_assert_impl( node->child_r_offset+sizeof(_Node) <= node_data_size );

		return (unsigned char*)node + sizeof(_Node);
	}

	unsigned char* child_l = (unsigned char*)node + sizeof(_Node);
	unsigned char* child_r = (unsigned char*)node + node->child_r_offset;
	unsigned char* end = child_l + node_data_size;

	void* ret = NULL;
	if ( !node->left_is_data ) {
		ret = _malloc_node( alloc_size, (_Node*)child_l, child_r-(child_l+sizeof(_Node)) );
		if ( ret != NULL ) return ret;
	}
	ret = _malloc_node( alloc_size, (_Node*)child_r, end-(child_r+sizeof(_Node)) );
	return ret;
}

static int _free_node( _Node* node_to_free, _Node* node_curr,size_t node_curr_data_size )
{
	if ( !node_curr->exists )
	{
		INVALID:
		fprintf( stderr, "Attempted to free invalid pointer 0x%p!\n", node_to_free );
		return -1;
	}

	if ( node_curr == node_to_free )
	{
		if ( !node_curr->left_is_data ) goto INVALID;
		node_curr->left_is_data = 0;
	}

	node_curr->
}

void __moss_heap_print( void )
{
}

void* malloc( size_t size )
{
	static int inited = 0;
	if ( !inited )
	{
		memset( _dyn_mem, 0x00, MOSS_MEM_SZ );
		inited = 1;
	}

	size += ( sizeof(_Node) - size%sizeof(_Node) ) % sizeof(_Node);
	if ( size == 0 ) return NULL;

	return _malloc_node( size, _dyn_mem,MOSS_MEM_SZ-sizeof(_Node) );
}
void free( void* ptr )
{
	if ( ptr == NULL ) return;

	_free_node(
		(_Node*)((unsigned char*)ptr-sizeof(_Node)),
		_dyn_mem, MOSS_MEM_SZ-sizeof(_Node)
	);
}

#endif



int       atoi ( char const* str )
{
	while ( isspace(*str) ) ++str;

	int is_neg = 0;
	if      ( *str == '+' )   ++str;
	else if ( *str == '-' ) { ++str; is_neg=1; }

	int neg_val = 0;                                              // Compute as negative to support
	for ( ; isdigit(*str); ++str ) neg_val=neg_val*10-(*str-'0'); // `INT_MIN`, etc.  Note subtract.

	return is_neg ? neg_val : -neg_val;
}
long      atol ( char const* str )
{
	while ( isspace(*str) ) ++str;

	int is_neg = 0;
	if      ( *str == '+' )   ++str;
	else if ( *str == '-' ) { ++str; is_neg=1; }

	long neg_val = 0;                                             // Compute as negative to support
	for ( ; isdigit(*str); ++str ) neg_val=neg_val*10-(*str-'0'); // `INT_MIN`, etc.  Note subtract.

	return is_neg ? neg_val : -neg_val;
}
long long atoll( char const* str )
{
	while ( isspace(*str) ) ++str;

	int is_neg = 0;
	if      ( *str == '+' )   ++str;
	else if ( *str == '-' ) { ++str; is_neg=1; }

	long long neg_val = 0;                                        // Compute as negative to support
	for ( ; isdigit(*str); ++str ) neg_val=neg_val*10-(*str-'0'); // `INT_MIN`, etc.  Note subtract.

	return is_neg ? neg_val : -neg_val;
}
