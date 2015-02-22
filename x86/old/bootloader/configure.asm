%ifndef __FILE_CONFIGURE__
%define __FILE_CONFIGURE__

%include "fail.asm"

enable_protected:
	cli
	lgdt  [gdtinfo]
	mov   eax, cr0
	or    al, 1
	mov   cr0, eax
	ret

gdtinfo:
	dw  gdt_end - gdt - 1  ;last byte in table
	dd  gdt                ;start of table

gdt dd 0,0  ;entry 0 is always unused
flatdesc db 0xff, 0xff, 0, 0, 0, 10010010b, 11001111b, 0
gdt_end:

%endif