%ifndef __FILE_LOAD__
%define __FILE_LOAD__

%include "defines.asm"
%include "print.asm"
%include "fail.asm"

%if 1
load:
	;Load to kernel
	PRINTSTR str_l_1, 0x07

	;MOV AX, 0
	;MOV ES, AX
	;MOV CL, kernel_sector   ;sector to be loaded
	;MOV AL, 30              ;!!!important number of sectors

	;call load_sector

	PRINTSTR str_l_2, 0x07

	ret

	str_l_1 db "Bootloader level 1: loading level 2",13,10,0
	str_l_2 db "Bootloader level 1: level 2 loaded!",13,10,0
load_sector:
	;MOV BX,0         ;offset from ES
	MOV BX, kernel_address
	MOV DL, drive      ;drive
	MOV DH, 0          ;head
	MOV CH, 0          ;track

	;read floppy/hard disk in CHS mode
	MOV AH, 2
	INT 0x13
	JC fail_load_sector

	ret
%endif

%if 0
load:                      ; Reset the hard drive
		PRINTSTR str_l_1, 0x07

		mov ax, 0           ;
		mov dl, 3           ; Drive=3 (=D)
		int 13h             ;

		jc load            ; ERROR => reset again
		PRINTSTR str_l_2, 0x07
load_sector:
		PRINTSTR str_l_3, 0x07
		mov ax, 1000h       ; ES:BX = 1000:0000
		mov es, ax          ;
		mov bx, 0           ;

		mov ah, 2           ; Load disk data to ES:BX
		mov al, 5           ; Load 5 sectors
		mov ch, 0           ; Cylinder=0
		mov cl, 2           ; Sector=2
		mov dh, 0           ; Head=0
		mov dl, 0           ; Drive=0
		int 13h             ; Read!

		jc load_sector      ; ERROR => Try again
		PRINTSTR str_l_2, 0x07

		jmp 1000h:0000      ; Jump to the program

		str_l_1 db "RESET",13,10,0
		str_l_2 db "SUCCESS",13,10,0
		str_l_3 db "LOAD SECTOR",13,10,0
%endif

%endif