#include "__arch.h"

#include <inttypes.h>
#include <stdint.h>

#include "__env.h"
#include "stdio.h"
#include "string.h"



#ifndef NDEBUG /* debug */

#if MOSS_ENV == MOSS_ENV_FREESTANDING || __INTELLISENSE__



/*
See also:
	https://wiki.osdev.org/Undefined_Behavior_Sanitization
	https://github.com/llvm/llvm-project/blob/main/compiler-rt/lib/ubsan/ubsan_handlers.h
	https://github.com/llvm/llvm-project/blob/main/compiler-rt/lib/ubsan/ubsan_handlers.cpp
	https://opensource.apple.com/source/xnu/xnu-7195.50.7.100.1/san/ubsan.h.auto.html
	https://gitlab.com/sortix/sortix/-/blob/master/libc/ubsan/ubsan.c
*/



#define UNRECOVERABLE(NAME, ...) \
	MOSS_NORET_ATTRIB void __ubsan_handle_##NAME(__VA_ARGS__);

#define RECOVERABLE(NAME, ...)\
	MOSS_NORET_ATTRIB void __ubsan_handle_##NAME(__VA_ARGS__);\
	MOSS_NORET_ATTRIB void __ubsan_handle_##NAME##_abort(__VA_ARGS__);



typedef uintptr_t ValueHandle;

struct SourceLocation
{
	char const* filename;
	uint32_t line  ;
	uint32_t column;
};

struct TypeDescriptor
{
	uint16_t type; // 0=integral, 1=floating-point, 0xffff=unknown

	union
	{
		struct { uint16_t is_signed : 1; uint16_t log2_bitsz : 15; } integral;
		struct { uint16_t log2_bitsz; } floating_point;
	}
	type_info;

	char const type_name[];
};

enum TypeCheckKind
{
	TCK_Load               , // operand of load
	TCK_Store              , // destination of store
	TCK_ReferenceBinding   , // ref binding
	TCK_MemberAccess       , // obj member access
	TCK_MemberCall         , // obj member function access
	TCK_ConstructorCall    ,
	TCK_DowncastPointer    , // obj downcast
	TCK_DowncastReference  ,
	TCK_Upcast             , // obj upcast
	TCK_UpcastToVirtualBase,
	TCK_NonnullAssign      , // check pointer assigned non-null value
	TCK_DynamicOperation     // `dynamic_cast`, `typeid`
};
static char const*const _g_type_check_strs[] =
{
	"load of", "store to", "reference binding to", "member access within", "member call on",
	"constructor call on", "downcast of", "downcast of", "upcast of", "cast to virtual base of",
	"_Nonnull binding to", "dynamic operation on"
};



struct _Report
{
	char buf[1024];
	int  buf_next;
};

static void _report_loc( struct _Report* report, struct SourceLocation const* loc )
{
	report->buf_next += sprintf( report->buf+report->buf_next, "[%s:(%u,%u)]",
		loc->filename, loc->line,loc->column
	);
}
__attribute__(( format(printf,2,3) ))
static void _report_printf( struct _Report* report, char const* fmt_cstr,... )
{
	va_list args;
	va_start( args, fmt_cstr );
	report->buf_next += vsprintf( report->buf+report->buf_next, fmt_cstr, args );
	va_end( args );
}
static void _report_type( struct _Report* report, struct TypeDescriptor const* type_descr )
{
	report->buf_next += sprintf( report->buf+report->buf_next, "%s",type_descr->type_name );
}
static void _report_init( struct _Report* report, struct SourceLocation const* loc )
{
	memset( report, 0x00, sizeof(struct _Report) );

	_report_loc   ( report, loc );
	_report_printf( report, " error: " );
}
MOSS_NORET_ATTRIB static void _report_issue( struct _Report const* report )
{
	__moss_kpanic( "%s",report->buf );
}



struct TypeMismatchData
{
	struct SourceLocation loc;
	struct TypeDescriptor const* type_descr;
	unsigned char log_alignment;
	enum TypeCheckKind check_kind;
};

struct AlignmentAssumptionData
{
	struct SourceLocation loc           ;
	struct SourceLocation loc_assumption;
	struct TypeDescriptor const* type_descr;
};

struct OverflowData
{
	struct SourceLocation loc;
	struct TypeDescriptor const* type_descr;
};

struct ShiftOutOfBoundsData
{
	struct SourceLocation loc;
	struct TypeDescriptor const* type_lhs;
	struct TypeDescriptor const* type_rhs;
};

struct OutOfBoundsData
{
	struct SourceLocation loc;
	struct TypeDescriptor const* type_arr;
	struct TypeDescriptor const* type_ind;
};

struct UnreachableData
{
	struct SourceLocation loc;
};

struct VLABoundData
{
	struct SourceLocation loc;
	struct TypeDescriptor const* type;
};

struct FloatCastOverflowDataV2
{
	struct SourceLocation loc;
	struct TypeDescriptor const* type_from;
	struct TypeDescriptor const* type_to  ;
};

struct InvalidValueData
{
	struct SourceLocation loc;
	struct TypeDescriptor const* type;
};

enum ImplicitConversionCheckKind //: unsigned char
{
	ICCK_IntegerTruncation                   = 0, // Legacy, was only used by clang 7.
	ICCK_UnsignedIntegerTruncation           = 1,
	ICCK_SignedIntegerTruncation             = 2,
	ICCK_IntegerSignChange                   = 3,
	ICCK_SignedIntegerTruncationOrSignChange = 4,
};
struct ImplicitConversionData
{
	struct SourceLocation loc;
	struct TypeDescriptor const* type_from;
	struct TypeDescriptor const* type_to  ;
	unsigned char kind; // `enum ImplicitConversionCheckKind`
};

enum BuiltinCheckKind //: unsigned char
{
	BCK_CTZPassedZero,
	BCK_CLZPassedZero
};
struct InvalidBuiltinData
{
	struct SourceLocation loc;
	unsigned char kind; // `BuiltinCheckKind`
};

struct PointerOverflowData
{
	struct SourceLocation loc;
};



MOSS_NORET_ATTRIB void __ubsan_handle_type_mismatch_v1(
	struct TypeMismatchData const* data, ValueHandle ptr
) {
	struct _Report report;
	_report_init( &report, &data->loc );

	if ( ptr == NULL )
	{
		_report_printf( &report, "%s null pointer of type ", _g_type_check_strs[data->check_kind] );
		_report_type  ( &report, data->type_descr );
		_report_printf( &report, "!\n" );
	}
	else
	{

		uintptr_t alignment = (uintptr_t)1 << data->log_alignment;
		if ( ptr & (alignment-1) )
		{
			_report_printf( &report, "%s misaligned address %p for type ",
				_g_type_check_strs[data->check_kind], (void*)ptr
			);
			_report_type  ( &report, data->type_descr );
			_report_printf( &report, ", which requires %" PRIu64 " byte alignment!\n", (uint64_t)alignment );
		}
		else
		{
			_report_printf( &report,
				"%s address %p with insufficient space for an object of type ",
				_g_type_check_strs[data->check_kind], (void*)ptr
			);
			_report_type  ( &report, data->type_descr );
			_report_printf( &report, "!\n" );
		}
	}

	_report_issue( &report );
}

MOSS_NORET_ATTRIB void __ubsan_handle_alignment_assumption(
	struct AlignmentAssumptionData const* data,
	ValueHandle ptr, ValueHandle alignment, ValueHandle offset
) {
	struct _Report report;
	_report_init  ( &report, &data->loc );

	_report_printf( &report, "assumption of %" PRIu64 " byte alignment", (uint64_t)alignment );

	if ( offset != 0 )
	{
		_report_printf( &report, " (with offset of %" PRIu64 " byte)", (uint64_t)offset );
	}
	_report_printf( &report, " for pointer of type " );
	_report_type  ( &report, data->type_descr );
	_report_printf( &report, " failed!\n" );

	if ( data->loc_assumption.filename != NULL )
	{
		_report_loc( &report, &data->loc_assumption );
		_report_printf( &report, ": note alignment assumption was specified here.\n" );
	}

	_report_issue( &report );
}

static void __ubsan_handle_overflow_common(
	struct OverflowData const* data, ValueHandle lhs,ValueHandle rhs, char const* op
) {
	bool is_signed = data->type_descr->type==0 && data->type_descr->type_info.integral.is_signed;
	if ( !is_signed ) return;

	struct _Report report;
	_report_init  ( &report, &data->loc );

	_report_printf( &report, "signed integer overflow: %s cannot be represented in type ", op );
	_report_type  ( &report, data->type_descr );
	_report_printf( &report, "!\n" );

	_report_issue ( &report );
}
void __ubsan_handle_add_overflow(
	struct OverflowData const* data, ValueHandle lhs,ValueHandle rhs
) {
	__ubsan_handle_overflow_common( data, lhs,rhs, "sum" );
}
void __ubsan_handle_sub_overflow(
	struct OverflowData const* data, ValueHandle lhs,ValueHandle rhs
) {
	__ubsan_handle_overflow_common( data, lhs,rhs, "difference" );
}
void __ubsan_handle_mul_overflow(
	struct OverflowData const* data, ValueHandle lhs,ValueHandle rhs
) {
	__ubsan_handle_overflow_common( data, lhs,rhs, "product" );
}

MOSS_NORET_ATTRIB void __ubsan_handle_negate_overflow(
	struct OverflowData const* data, ValueHandle old_val
) {
	bool is_signed = data->type_descr->type==0 && data->type_descr->type_info.integral.is_signed;

	struct _Report report;
	_report_init( &report, &data->loc );

	if ( is_signed )
	{
		_report_printf( &report, "negation overflows in type " );
	}
	else
	{
		_report_printf( &report, "negation cannot be represented in type " );
	}
	_report_type  ( &report, data->type_descr );
	_report_printf( &report, "!\n" );

	_report_issue ( &report );
}

MOSS_NORET_ATTRIB void __ubsan_handle_divrem_overflow(
	struct OverflowData const* data, ValueHandle lhs,ValueHandle rhs
) {
	struct _Report report;
	_report_init  ( &report, &data->loc );

	_report_printf( &report, "division cannot be represented in type " );
	_report_type  ( &report, data->type_descr );
	_report_printf( &report, "!\n" );

	_report_issue ( &report );
}

MOSS_NORET_ATTRIB void __ubsan_handle_shift_out_of_bounds(
	struct ShiftOutOfBoundsData const* data, ValueHandle lhs,ValueHandle rhs
) {
	struct _Report report;
	_report_init  ( &report, &data->loc );

	_report_printf( &report, "invalid shift!\n" );

	_report_issue ( &report );
}

MOSS_NORET_ATTRIB void __ubsan_handle_out_of_bounds(
	struct OutOfBoundsData const* data, ValueHandle index
) {
	struct _Report report;
	_report_init  ( &report, &data->loc );

	_report_printf( &report, "index out of bounds for type " );
	_report_type  ( &report, data->type_arr );
	_report_printf( &report, "!\n" );

	_report_issue ( &report );
}

MOSS_NORET_ATTRIB void __ubsan_handle_builtin_unreachable(
	struct UnreachableData const* data
) {
	struct _Report report;
	_report_init  ( &report, &data->loc );

	_report_printf( &report, "execution reached an unreachable program point!\n" );

	_report_issue ( &report );
}

//UNRECOVERABLE( missing_return, struct UnreachableData const* data )

//RECOVERABLE( vla_bound_not_positive, struct VLABoundData const* data, ValueHandle bound )

MOSS_NORET_ATTRIB void __ubsan_handle_float_cast_overflow(
	struct FloatCastOverflowDataV2 const* data, ValueHandle from
) {
	struct _Report report;
	_report_init  ( &report, &data->loc );

	_report_printf( &report, " conversion outside the range of representable values for type " );
	_report_type  ( &report, data->type_to );
	_report_printf( &report, "!\n" );

	_report_issue ( &report );
}

MOSS_NORET_ATTRIB void __ubsan_handle_load_invalid_value(
	struct InvalidValueData const* data, ValueHandle val
) {
	struct _Report report;
	_report_init  ( &report, &data->loc );

	_report_printf( &report, " load of value, which is not a valid value for type " );
	_report_type  ( &report, data->type );
	_report_printf( &report, "!\n" );

	_report_issue ( &report );
}

MOSS_NORET_ATTRIB void __ubsan_handle_implicit_conversion(
	struct ImplicitConversionData const* data, ValueHandle src,ValueHandle dst
) {
	struct _Report report;
	_report_init  ( &report, &data->loc );

	_report_printf( &report, " implicit conversion from type " );
	_report_type  ( &report, data->type_from );
	_report_printf( &report, " to type " );
	_report_type  ( &report, data->type_to   );
	_report_printf( &report, " changed the value!\n" );

	_report_issue ( &report );
}

MOSS_NORET_ATTRIB void __ubsan_handle_invalid_builtin(
	struct InvalidBuiltinData const* data
) {
	struct _Report report;
	_report_init  ( &report, &data->loc );

	_report_printf( &report, " passing zero to %s, which is not a valid argument!",
		data->kind==BCK_CTZPassedZero ? "ctz()" : "clz()"
	);

	_report_issue ( &report );
}

//RECOVERABLE( invalid_objc_cast, struct InvalidObjCCast const* data, ValueHandle ptr )

//RECOVERABLE( nonnull_return_v1, struct NonNullReturnData const* data, struct SourceLocation const *loc )
//RECOVERABLE( nullability_return_v1, struct NonNullReturnData const* data, struct SourceLocation const *loc )

//RECOVERABLE( nonnull_arg, struct NonNullArgData const* data )
//RECOVERABLE( nullability_arg, struct NonNullArgData const* data )

MOSS_NORET_ATTRIB void __ubsan_handle_pointer_overflow(
	struct PointerOverflowData const* data, ValueHandle base, ValueHandle result
) {
	struct _Report report;
	_report_init  ( &report, &data->loc );

	if      ( base==0 && result==0 )
	{
		_report_printf( &report, " applying zero offset to null pointer!" );
	}
	else if ( base==0 && result!=0 )
	{
		_report_printf( &report, " applying non-zero offset %" PRIu64 " to null pointer!",
			(uint64_t)result
		);
	}
	else if ( base!=0 && result==0 )
	{
		_report_printf( &report,
			" applying non-zero offset to non-null pointer %p produced null pointer!",
			(void*)base
		);
	}
	else
	{
		_report_printf( &report, " pointer overflow!" );
	}

	_report_issue ( &report );
}


#if 0

/// \brief Known CFI check kinds.
/// Keep in sync with the enum of the same name in CodeGenFunction.h
enum CFITypeCheckKind : unsigned char
{
	CFITCK_VCall,
	CFITCK_NVCall,
	CFITCK_DerivedCast,
	CFITCK_UnrelatedCast,
	CFITCK_ICall,
	CFITCK_NVMFCall,
	CFITCK_VMFCall,
};

struct CFICheckFailData
{
	CFITypeCheckKind CheckKind;
	SourceLocation Loc;
	const TypeDescriptor &Type;
};

/// \brief Handle control flow integrity failures.
RECOVERABLE(cfi_check_fail, CFICheckFailData *Data, ValueHandle Function,
			uptr VtableIsValid)

struct ReportOptions;

extern "C" SANITIZER_INTERFACE_ATTRIBUTE void __ubsan_handle_cfi_bad_type(
	CFICheckFailData *Data, ValueHandle Vtable, bool ValidVtable,
	ReportOptions Opts);

struct FunctionTypeMismatchData
{
	SourceLocation Loc;
	const TypeDescriptor &Type;
};

extern "C" SANITIZER_INTERFACE_ATTRIBUTE void
__ubsan_handle_function_type_mismatch(FunctionTypeMismatchData *Data,
									  ValueHandle Val);
extern "C" SANITIZER_INTERFACE_ATTRIBUTE void
__ubsan_handle_function_type_mismatch_abort(FunctionTypeMismatchData *Data,
											ValueHandle Val);
}

void __ubsan_handle_type_mismatch_v1(TypeMismatchData *Data, ValueHandle Pointer)
{
}

#endif



#endif

#endif
