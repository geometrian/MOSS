ENTRY(_start)

/* these are configurable for level 2 loaders */
mmio        = 0xfffffffff8000000;
fb          = 0xfffffffffc000000;
bootboot    = 0xffffffffffe00000;
environment = 0xffffffffffe01000;

/* initstack = 16384; */
initstack = 16384;

PHDRS
{
	/* one single loadable segment */
	boot PT_LOAD;
}
SECTIONS
{
	. = 0xffffffffffe02000;
	.text :
	{
		KEEP(*(.text.boot)) *(.text .text.*)   /* code */
		*(.rodata .rodata.*)                   /* data */
		*(.data .data.*)
	} :boot
	.bss (NOLOAD) : /* bss */
	{
		. = ALIGN(16);
		*(.bss .bss.*)
		*(COMMON)
	} :boot

	/DISCARD/ : { *(.eh_frame) *(.comment) }
}
