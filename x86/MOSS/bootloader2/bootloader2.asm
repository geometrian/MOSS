[BITS 32]
;No ORG when not compiled into .bin directly
[global loader]
[extern kernel_main]  ; defined in kernel.cpp

; reserve initial kernel stack space
STACKSIZE equ 0x4000                    ; that's 16k.

jmp  loader

%include "print.asm"
%include "print_level2.asm"

loader:
	PRINT  str_1, 0x7, 0x0

	mov  esp, stack + STACKSIZE  ; set up the stack

	;call  kernel_main            ; call kernel

	.hang:
		jmp  .hang

	str_1 db "Bootloader level 2!",0

section .bss

align 4
stack:  resb STACKSIZE                   ; reserve 16k stack on a doubleword boundary