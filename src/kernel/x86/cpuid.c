#include "cpuid.h"



uint32_t x86_cpuid_is_supported(void)
{
	/*
	See if we are able to change the 'ID' bit in the 'EFLAGS' register.  If we can, then the `cpuid`
	instruction is supported.  (Note: 'FLAGS' was the 16-bit version of the same register, while
	'RFLAGS' is the 64-bit version (the upper half is all reserved).

	https://wiki.osdev.org/CPUID#Checking_CPUID_availability
	https://en.wikipedia.org/wiki/FLAGS_register
	*/

	#if   defined __MOSS_32__
		uint32_t eax;
		asm(
			// Save 'EFLAGS' register of the caller on the stack`
			"pushfd\n"
			// Toggle the 'ID' bit in 'EFLAGS'
			"pushfd\n"
			"xor     dword ptr [esp], 0x00200000\n"
			"popfd\n"
			// Move (toggled) 'EFLAGS' into 'eax'
			"pushfd\n"
			"pop     eax\n"
			// Compare with the 'EFLAGS' of the caller's stored on the stack.  If `eax` is zero after
			// this, then the 'ID' bit can't be changed (i.e. `cpuid` unsupported).  Else, it is
			// 0x00200000 and `cpuid` is supported.
			"xor     eax, [esp]\n"
			// Restore caller's 'EFLAGS'
			"popfd\n"
			: "=a"(eax)
			:
			: "cc"
		);
		return eax;
	#elif defined __MOSS_64__
		uint64_t rax;
		asm(
			// Save 'RFLAGS' register of the caller on the stack`
			"pushfq\n"
			// Toggle the 'ID' bit in 'RFLAGS'
			"pushfq\n"
			"xor     dword ptr [rsp], 0x00200000\n"
			"popfq\n"
			// Move (toggled) 'RFLAGS' into 'rax'
			"pushfq\n"
			"pop     rax\n"
			// Compare with the 'RFLAGS' of the caller's stored on the stack.  If `rax` is zero after
			// this, then the 'ID' bit can't be changed (i.e. `cpuid` unsupported).  Else, it is
			// 0x00200000 and `cpuid` is supported.
			"xor     rax, [rsp]\n"
			// Restore caller's 'RFLAGS'
			"popfq\n"
			: "=a"(rax)
			:
			: "cc"
		);
		return (uint32_t)rax;
	#else
	#endif

}

CPUID0 x86_cpuid_0(void)
{
	//https://en.wikipedia.org/wiki/CPUID#EAX=0:_Highest_Function_Parameter_and_Manufacturer_ID
	//	Gets largest `cpuid` basic-info `eax` argument into `eax`.  Also returns manufacturer string
	//	in `ebx`, `edx`, `ecx` (in that order)
	CPUID0 ret;

	CPUIDi tmp = x86_cpuid_i_1arg( 0 );

	ret.max_leaf = tmp.eax;
	*(uint32_t*)(ret.manufacturer  ) = tmp.ebx;
	*(uint32_t*)(ret.manufacturer+4) = tmp.edx;
	*(uint32_t*)(ret.manufacturer+8) = tmp.ecx;
	ret.manufacturer[12] = '\0';

	return ret;
}
CPUID1 x86_cpuid_1(void)
{
	//https://en.wikipedia.org/wiki/CPUID#EAX=1:_Processor_Info_and_Feature_Bits
	CPUID1 ret;

	CPUIDi tmp = x86_cpuid_i_1arg( 1 );

	ret.has_packed[ 0 ] = tmp.ecx;
	ret.has_packed[ 1 ] = tmp.edx;

	{
		typedef struct
		{
			uint32_t stepping_id   : 4;
			uint32_t model_id      : 4;
			uint32_t family_id     : 4;
			uint32_t proc_type     : 2;
			uint32_t               : 2;
			uint32_t model_id_ext  : 4;
			uint32_t family_id_ext : 8;
			uint32_t               : 4;
		}
		CPU_Info1;

		static_assert( sizeof(CPU_Info1) == sizeof(uint32_t) );
		union { CPU_Info1 cpu_info; uint32_t eax; } u;
		u.eax = tmp.eax;

		ret.ident.step = u.cpu_info.stepping_id;
		ret.ident.model = u.cpu_info.model_id;
		if ( u.cpu_info.family_id==6 || u.cpu_info.family_id==15 )
		{
			ret.ident.model |= u.cpu_info.family_id_ext << 4;
		}
		ret.ident.family = u.cpu_info.family_id;
		if ( u.cpu_info.family_id == 15 )
		{
			//The wording of the spec suggests this doesn't overflow, but it's unclear
			ret.ident.family += u.cpu_info.family_id_ext;
		}
		ret.ident.proc_type = u.cpu_info.proc_type;
	}

	{
		typedef struct
		{
			uint8_t brand_index       ;
			uint8_t opt_icache_line_sz;
			uint8_t hyperthread_count;
			uint8_t hyperthread_ind  ;
		}
		CPU_Info2;

		static_assert( sizeof(CPU_Info2) == sizeof(uint32_t) );
		union { CPU_Info2 cpu_info; uint32_t ebx; } u;
		u.ebx = tmp.ebx;

		ret.ident.brand_index = u.cpu_info.brand_index;
		ret.icache_line_sz = ret.has.isa_clflush ? u.cpu_info.opt_icache_line_sz : 0;
		if ( tmp.edx & (1<<28) ) //HTT
		{
			ret.hyperthread_count = u.cpu_info.hyperthread_count;
			ret.hyperthread_ind   = u.cpu_info.hyperthread_ind  ;
		}
		else
		{
			ret.hyperthread_count =   0;
			ret.hyperthread_ind   = 255;
		}
	}

	return ret;
}

CPUID_Info x86_cpuid_all(void)
{
	CPUID_Info ret;
	memset( &ret, 0x00, sizeof(CPUID_Info) );

	CPUID0 tmp0 = x86_cpuid_0();
	memcpy( &ret.manufacturer,tmp0.manufacturer, 12+1 );

	if ( tmp0.max_leaf < 1 ) return ret;

	ret.cpu_info.is_valid = true;
	ret.cpu_info.data = x86_cpuid_1();

	return ret;
}
