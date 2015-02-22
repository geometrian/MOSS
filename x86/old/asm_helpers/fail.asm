%ifndef __FILE_FAIL__
%define __FILE_FAIL__

%include "print.asm"

fail_load_returned:
	call fail_general

	PRINTSTR1 str_flr_1, 0x04
	jmp fail_hang

	str_flr_1 db "Load returned!",13,10,0
fail_kernel_returned:
	call fail_general

	PRINTSTR1 str_fkr_1, 0x04
	jmp fail_hang

	str_fkr_1 db "Kernel returned!",13,10,0
fail_load_sector:
	call fail_general

	PRINTSTR1 str_fls_1, 0x04
	jmp fail_hang

	str_fls_1 db "Load sector failed!",13,10,0
fail_a20:
	call fail_general

	PRINTSTR1 str_fa20_1, 0x04
	jmp fail_hang

	str_fa20_1 db "Could not enable the a20 address line!",13,10,0

fail_general:
	PRINTSTR1 str_fg_1, 0x04
	ret

	str_fg_1 db "Loading level 2 failed!",13,10,"    ",0
fail_hang:
	jmp  $

%endif