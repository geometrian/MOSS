[BITS 16]
[ORG 0x7C00]

;BIOS loads code to 0x0000:0x7C00, but CS is not necessarily 0x7C00/16 = 0x7C0
;So, jump to a known address: CS=0x0000 and IP=0x7C00 and set up a new stack immediately
jmp  0x0000:main

%include "../asm_helpers/print.asm"
%include "print_level1.asm"

; PRINT 3 is a macro that prints given a string, text color, and background color
%macro PRINT1 3  ; print a level 1 text (i.e. something like "Bootloader level 1: <text>")
	call   print_level1
	PRINT  %1, %2, %3
%endmacro
%macro PRINTI 1  ; print an info line (i.e. PRINT1, but without having to specify a dark grey color)
	mov   esi, %1
	call  print_info
%endmacro
%macro FAIL 2  ; a failure condition--print an error and hang
	mov  esi, %2
	%1   fail
%endmacro

main:
	;Setup segment registers
	;	Setup stack
	mov  ax, 0x0000
	mov  ss, ax
	mov  sp, 0x7C00 ; Stack grows down from offset 0x7C00 toward 0x0000.
	;	Setup ES to point to text video memory
	mov  ax, 0xB800
	mov  es, ax
	;	The DS should be 0x0000.  Apparently it is by default, but set it to that just to be sure.
	mov  ax, 0x0000
	mov  ds, ax

	mov  [bootdisk], dl  ; boot drive number given by the bios in dl

	;Print splash info
	PRINT  str_ss_1, 0x0, 0x2

	CURSOR_ADD_Y 2

	;Enable A20 line
	mov   ax, 0x2401     ; 0x2400 to disable
	int   0x15
	FAIL  jc, str_f_a20

	;Load kernel
	PRINTI  str_lk_1
	;	Check support INT13h extensions.  Implies that LBA addressing MAY be available
	mov     ah, 0x41
	mov     bx, 0x55AA
	mov     dl, 0x80
	int     0x13
	FAIL    jc, str_f_int13
	PRINTI  str_lk_2
	;	Do the transfer.  See http://en.wikipedia.org/wiki/INT_13H#INT_13h_AH.3D42h:_Extended_Read_Sectors_From_Drive
	PRINTI  str_lk_3
	mov     si, DAP
	mov     ah, 0x42
	mov     dl, [bootdisk]
	int     0x13
	PRINTI  str_lk_4
	FAIL    jc, str_f_load
	;	Jump to kernel
	jmp     0x0000:0x8000

	;Kernel returned
	FAIL    jmp, str_f_kr

print_info:
	push  esi
	call  print_level1
	pop   esi
	SET_COLOR_L(0x8,0x0)
	call  print_string
	ret
fail:
	push  esi
	call  print_level1
	pop   esi
	SET_COLOR_L(0x4,0x0)
	call  print_string
	jmp   $

DAP:            ; Disk Address Packet
	;Size of DAP
	db  0x10
	;Unused
	db  0
	;Number of sectors to read
	dw  1       ; int 13 resets this to # of blocks actually read/written
	;Offset and segment of destination
	;db  0x00    ; offset
	;db  0x7C
	;db  0x00    ; segment
	;db  0x00
	dw  0x8000  ; offset
	dw  0x0000  ; segment
	;LBA of start
	dd  1       ; lower 4 bytes
	dd  0       ; upper 4 bytes

bootdisk  db 0

;ASCII 13 is \r, ASCII 10 is \n
str_ss_1  db "MOSS 0.1 Bootloader 1",13,10,"Ian Mallett",13,10,0

str_lk_1  db "load level 2:",13,10,0
str_lk_2  db "INT13 supported!",13,10,0
str_lk_3  db "load start:",13,10,0
str_lk_4  db "load done!",13,10,0

str_f_kr     db "kernel returned!",0
str_f_a20    db "no A20!",0
str_f_int13  db "no INT13!",0
str_f_load   db "load failed!",0

;Fill the rest of sector with 0
TIMES  510 - ($ - $$) db 0

;Add boot signature at the end of bootloader
DW 0xAA55