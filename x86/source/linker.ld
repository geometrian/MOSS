/*
The bootloader will look at this image and start execution
at the symbol designated as the entry point.
*/
ENTRY(_start)

/*
Tell where the various sections of the object files will be put in the final kernel image.
*/
SECTIONS
{
	/*
	Begin putting sections at 1 MiB, a conventional place for kernels to be loaded at by the bootloader.
	*/
	. = 1M;

	/*
	All of these sections used to be 4K.  MOSS is going to use the first 1MiB
	to store these, so they're spread out equally.  The heap will start at 2MiB.
	*/

	/*
	First put the multiboot header, as it is required to be put very
	early in the image or the bootloader won't recognize the file format.
	Next, the .text section.
	*/
	.text BLOCK(256K) : ALIGN(4K)
	{
		*(.multiboot)
		*(.text)
	}

	/*
	Read-only data.
	*/
	.rodata BLOCK(256K) : ALIGN(4K)
	{
		*(.rodata)
	}

	/*
	Read-write data (initialized)
	*/
	.data BLOCK(256K) : ALIGN(4K)
	{
		*(.data)
	}

	/*
	Read-write data (uninitialized) and stack
	*/
	.bss BLOCK(256K) : ALIGN(4K)
	{
		*(COMMON)
		*(.bss)
		*(.bootstrap_stack)
	}

	/*
	The compiler may produce other sections, by default it will put them in a
	segment with the same name. Simply add stuff here as needed.
	*/
}