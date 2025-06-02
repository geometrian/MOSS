#pragma once

#include "../stdafx.h"



/*
CPUID is an x86 instruction that gets information about the CPU.  See:
	https://en.wikipedia.org/wiki/CPUID
	https://www.felixcloutier.com/x86/cpuid
*/



typedef struct
{
	uint32_t eax, ebx, ecx, edx;
}
CPUIDi;

//`cpuid` leaf `eax`=0: max leaf `eax` and manufacturer ID string
typedef struct
{
	uint32_t max_leaf;

	__attribute__((aligned(4)))
	char manufacturer[ 12 + 1 ];
}
CPUID0;

//`cpuid` leaf `eax`=1: CPU info
typedef struct
{
	uint8_t step  ; //4-bit
	uint8_t model ; //4-bit or 8-bit
	uint8_t family; //unclear; probably 8-bit, but maybe 9-bit
	uint8_t proc_type; //2-bit, 00=OEM, 01=IntelOverdrive, 10=P5 Pentium dual-core, 11=reserved
	uint8_t brand_index;
}
CPUID1_Ident;
typedef struct //in order `ecx`, `edx`
{
	uint32_t isa_sse3            : 1; // SSE3 instructions
	uint32_t isa_pclmulqdq       : 1; // `pclmulqdq` (carry-less multiply)
	uint32_t ds_is_64bit         : 1; // 64-bit debug store
	uint32_t isa_monitor         : 1; // `monitor` and `mwait`
	uint32_t ds_cpl              : 1; // debug store is CPL-qualified
	uint32_t vmx                 : 1; // virtual-machine extensions
	uint32_t smx                 : 1; // safer mode extensions
	uint32_t est                 : 1; // enhanced SpeedStep

	uint32_t thermal_by_slow     : 1; // thermal control 2 (works by slowing down)
	uint32_t isa_ssse3           : 1; // SSSE3/SSE3S (supplemental SSE3 instructions)
	uint32_t L1_ctx_id           : 1; // L1 context ID
	uint32_t sdbg                : 1; // Silicon debug interface
	uint32_t isa_fma3            : 1; // FMA3 instructions
	uint32_t isa_cmp_and_swap_16 : 1; // `cmpxchg16b` (compare and swap)
	uint32_t xtpr                : 1; // disable sending task priority messages
	uint32_t pdcm                : 1; // Perfmon and debug

	uint32_t                     : 1; // (reserved)
	uint32_t proc_ctx_id         : 1; // process context identifiers
	uint32_t dma_direct_cache    : 1; // direct cache access for DMA writes
	uint32_t isa_sse41           : 1; // SSE4.1
	uint32_t isa_sse42           : 1; // SSE4.2
	uint32_t x2apic              : 1; // x2APIC (enhanced APIC)
	uint32_t isa_movbe           : 1; // `movbe`
	uint32_t isa_popcnt          : 1; // `popcnt`

	uint32_t tsc_deadline        : 1; // TSC deadline value
	uint32_t isa_aes             : 1; // AES instructions
	uint32_t isa_xsave           : 1; // `xsave`, `xrstor`, `xsetbv`, `xgetbv` (extensible processor state save / restore)
	uint32_t isa_xsave_by_os     : 1; // `xsave` enabled by OS
	uint32_t isa_avx             : 1; // AVX instructions
	uint32_t isa_f16c            : 1; // conversion instructions from / to float16
	uint32_t rdrand              : 1; // on-chip RNG with `rdrand`
	uint32_t hypervisor          : 1; // hypervisor present

	uint32_t x87_fpu             : 1; // onboard x87 FPU
	uint32_t vme                 : 1; // virtual 8086 mode extensions
	uint32_t debug_ext           : 1; // debugging extensions
	uint32_t page_sz_ext         : 1; // page size extension
	uint32_t tsc                 : 1; // timestamp counter, `rdtsc`
	uint32_t msr                 : 1; // model-specific registers
	uint32_t pae                 : 1; // physical address extension
	uint32_t mce                 : 1; // machine check exception

	uint32_t isa_cmp_and_swap_8  : 1; // `cmpxchg8b` (compare and swap)
	uint32_t apic                : 1; // advanced programmable interrupt controller
	uint32_t                     : 1; // (reserved)
	uint32_t isa_fast_syscall    : 1; // `sysenter` and `sysexit` instructions
	uint32_t mtrr                : 1; // memory type range registers
	uint32_t page_enable         : 1; // page enable bit in 'CR4'
	uint32_t mca                 : 1; // machine check architecture error reporting
	uint32_t isa_cmov            : 1; // `cmov`, `fcmov`, `fcomi`

	uint32_t page_attrib_table   : 1; // page attribute table
	uint32_t page_sz_ext_36      : 1; // 36-bit page size extension
	uint32_t proc_serial_num     : 1; // processor serial number (both supported and enabled)
	uint32_t isa_clflush         : 1; // `clflush`, `clflushopt` SSE2 cacheline flush
	uint32_t ia64_nx             : 1; // no-execute bit on IA-64 (Itanium)
	uint32_t ds                  : 1; // debug store
	uint32_t acpi                : 1; // MSRs for ACPI
	uint32_t isa_mmx             : 1; // MMX instructions

	uint32_t isa_fxsr            : 1; // `fxsave`, `fxrstor`
	uint32_t isa_sse             : 1; // SSE instructions (including `sfence`)
	uint32_t isa_sse2            : 1; // SSE2 instructions (including `lfence`, `mfence`)
	uint32_t cache_self_snoop    : 1; // cache self-snoop
	uint32_t                     : 1; // (used internally)
	uint32_t thermal_by_wait     : 1; // thermal control 1 (works by inserting wait cycle)
	uint32_t ia64_emul_x86       : 1; // IA-64 processor emulating x86
	uint32_t pbe                 : 1; // pending break enable
}
CPUID1_Has;
static_assert( sizeof(CPUID1_Has) == 2*sizeof(uint32_t) );
typedef struct
{
	CPUID1_Ident ident;

	uint8_t icache_line_sz; //0 if unknown
	uint8_t hyperthread_count; //  0 if unknown
	uint8_t hyperthread_ind  ; //255 if unknown

	union
	{
		CPUID1_Has has;
		uint32_t   has_packed[2];
	};
}
CPUID1;



//Return type is semantically `bool`, with false=0 and true=0x00200000
uint32_t x86_cpuid_is_supported(void);

// On some older processors, `eax` > 1 may leave `ebx` and `ecx` unmodified, so in the following we
// constrain them to zero so their values are known.
MOSS_ND_INLINE CPUIDi x86_cpuid_i_1arg( uint32_t eax               )
{
	CPUIDi ret;
	asm(
		"cpuid\n"
		: "=a"(ret.eax), "=b"(ret.ebx), "=c"(ret.ecx), "=d"(ret.edx)
		: "a"(eax), "b"(0), "c"(0)
		:
	);
	return ret;
}
MOSS_ND_INLINE CPUIDi x86_cpuid_i_2arg( uint32_t eax, uint32_t ecx )
{
	CPUIDi ret;
	asm(
		"xor   ebx, ebx\n"
		"cpuid\n"
		: "=a"(ret.eax), "=b"(ret.ebx), "=c"(ret.ecx), "=d"(ret.edx)
		: "a"(eax), "b"(0), "c"(ecx)
		:
	);
	return ret;
}

CPUID0 x86_cpuid_0(void);
CPUID1 x86_cpuid_1(void);

typedef struct
{
	__attribute__((aligned(4)))
	char manufacturer[ 12 + 1 ];

	struct { bool is_valid; CPUID1 data; } cpu_info;
}
CPUID_Info;
CPUID_Info x86_cpuid_all(void);
