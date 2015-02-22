[BITS 16]
[ORG 0x7C00]
jmp  0x0000:main

%include "print.asm"
%include "fail.asm"
%include "load_kernel.asm"
%include "configure.asm"

main:
	;Set video mode to 80 x 25, 16 colors
	mov  al, 0x03
	mov  ah, 0x00
	int  0x10

	;Print splash screen info
	PRINTSTR str_ss_1, 0x06
	PRINTSTR str_ss_2, 0x02

	;Enable A20 line
	mov  ax, 0x2401  ;0x2400 to disable
	int  0x15
	jc   fail_a20

	;Load to kernel
	PRINTSTR1 str_lk_1, 0x07
	call load_kernel
	PRINTSTR1 str_lk_2, 0x07
	PRINTSTR1 str_lk_3, 0x06

	call fail_load_returned

str_ss_1 db "MOSS v. 0.1 Stage 1 Bootloader",13,10,0
str_ss_2 db "Ian Mallett - 2012",13,10,10,0

str_lk_1 db "loading level 2",13,10,0
str_lk_2 db "level 2 loaded!",13,10,0
str_lk_3 db "Jumping to kernel . . .",13,10,0

;Fill the rest of sector with 0
TIMES 510 - ($ - $$) db 0

;Add boot signature at the end of bootloader
DW 0xAA55