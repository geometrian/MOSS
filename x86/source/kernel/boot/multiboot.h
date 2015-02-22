namespace MOSS { namespace Boot {


#define MULTIBOOT_HEADER_MAGIC 0x1BADB002

#ifdef __ELF__
	#define MULTIBOOT_HEADER_FLAGS 0x00000003
#else
	#define MULTIBOOT_HEADER_FLAGS 0x00010003
#endif

#define MULTIBOOT_BOOTLOADER_MAGIC 0x2BADB002

#define STACK_SIZE 0x4000

/* C symbol format. HAVE_ASM_USCORE is defined by configure. */
#ifdef HAVE_ASM_USCORE
	#define EXT_C(sym) _##sym
#else
	#define EXT_C(sym) sym
#endif

#ifndef ASM
/* Do not include here in boot.S. */

class multiboot_header_t { public:
	unsigned long magic;
	unsigned long flags;
	unsigned long checksum;
	unsigned long header_addr;
	unsigned long load_addr;
	unsigned long load_end_addr;
	unsigned long bss_end_addr;
	unsigned long entry_addr;
};

class aout_symbol_table_t { public:
	unsigned long tabsize;
	unsigned long strsize;
	unsigned long addr;
	unsigned long reserved;
};

class elf_section_header_table_t { public:
	unsigned long num;
	unsigned long size;
	unsigned long addr;
	unsigned long shndx;
};

class multiboot_info_t { public:
	unsigned long flags;
	unsigned long mem_lower;
	unsigned long mem_upper;
	unsigned long boot_device;
	unsigned long cmdline;
	unsigned long mods_count;
	unsigned long mods_addr;
	union {
		aout_symbol_table_t aout_sym;
		elf_section_header_table_t elf_sec;
	} u;
	unsigned long mmap_length;
	unsigned long mmap_addr;
};

class module_t { public:
	unsigned long mod_start;
	unsigned long mod_end;
	unsigned long string;
	unsigned long reserved;
};

//Be careful that the offset 0 is base_addr_low but no size.
class memory_map_t { public:
	unsigned long size;
	unsigned long base_addr_low;
	unsigned long base_addr_high;
	unsigned long length_low;
	unsigned long length_high;
	unsigned long type;
};

#endif /* ! ASM */


}}