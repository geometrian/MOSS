%ifndef __FILE_LOAD__
%define __FILE_LOAD__

%include "defines.asm"
%include "print.asm"
%include "fail.asm"

load_kernel:
	mov  si, DAPACK  ; address of "disk address packet"
	mov  ah, 0x42    ; AL is unused
	mov  dl, 0x80    ; drive number 0 (OR the drive # with 0x80)
	int  0x13
	jc   fail_load_sector
	ret

DAPACK:
	db  0x10
	db  0
blkcnt:
	dw  16      ; int 13 resets this to # of blocks actually read/written
db_add:
	dw  0x8000  ; memory buffer destination address (0x0000:<the number>)
	dw  0       ; in memory page zero
d_lba:
	dd  1       ; put the lba to read in this spot
	dd  0       ; more storage bytes only for big lba's ( > 4 bytes )

%endif