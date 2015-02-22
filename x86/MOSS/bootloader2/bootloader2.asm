[BITS 32]
;No ORG when not compiled into .bin directly
[global loader]
[extern kernel_main]  ; defined in kernel.cpp

jmp  loader

%include "print.asm"
%include "print_level2.asm"

STACK_SIZE EQU 0x4000 ;16K

loader:
	PRINT  str_1, 0x7, 0x0

	mov  esp, stack+STACK_SIZE

	;call  kernel_main       ; call kernel

	.hang:
		jmp  .hang

	str_1 db "Bootloader level 2!",0

section .bss

align 4
stack:  resb STACK_SIZE  ; reserve stack on a doubleword boundary