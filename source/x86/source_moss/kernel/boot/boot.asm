;Constants for a multiboot header
MBALIGN   equ 1<<0              ;Align loaded modules on page boundaries
MEMINFO   equ 1<<1              ;Provide memory map
FLAGS     equ MBALIGN | MEMINFO ;This is the Multiboot "flag" field
MAGIC     equ 0x1BADB002        ;"magic number" lets bootloader find the header
CHECKSUM  equ -(MAGIC + FLAGS)  ;Checksum of above, to prove we are multiboot

;Declare a header as in the Multiboot standard.  We put this into a special
;	section so we can force the header to be in the start of the final program.
;	The bootloader will search for this magic sequence and recognize us as a
;	multiboot kernel.
section .multiboot
align 4
	dd MAGIC
	dd FLAGS
	dd CHECKSUM

;Currently the stack pointer register (esp) points at anything and using it may
;	cause massive harm.  Instead, provide our own stack.  Allocate room for a
;	small temporary stack by creating a symbol at the bottom of it, allocating
;	16,384 bytes for it, and finally creating a symbol at the top.
section .bootstrap_stack
align 4
stack_bottom:
	times 16384 db 0
stack_top:

;The linker script specifies _start as the entry point to the kernel and the
;	bootloader will jump to this position once the kernel has been loaded.
section .text
global _start
_start:
	;Welcome to the kernel!  There is now sufficient code for the bootloader to
	;	load and run MOSS.  The next task is preparing the processor for execution
	;	of the C and C++ code the kernel is written in.  C doesn't expect much at
	;	this point and we only need to set up a stack.
	;Note: the processor is still not fully initialized yet and e.g. floating point
	;	instructions will not work.

	;To set up a stack, we simply set the esp register to point to the top of
	;	our stack (as it grows downwards).
	mov  esp, stack_top

	;Push the pointer to the Multiboot information structure.
	push  ebx
	;Push the magic value.
	push  eax

	;We are now ready to actually execute C code.  Jump to C entry point called
	;	"kernel_entry".
	extern kernel_entry
	call  kernel_entry

	;In case the function returns (which it shouldn't), we can do nothing except
	;	stop entirely.
	extern die
	call  die
.hang:
	jmp  .hang