;MOSS bootloader
;Ian Mallett

ADDRESS_KERNEL  EQU 0x1000      ; Address that the kernel will be loaded to by the bootloader
SEGMENT_ID_CODE EQU 0x0008      ; Code segment ID is 8
SEGMENT_ID_DATA EQU 0x0010      ; Data segment ID is 16
ADDRESS_STACK   EQU 0x00090000  ; Top of conventional memory

[BITS 16]
[ORG 0x7C00]

;BIOS loads code to 0x0000:0x7C00, but CS is not necessarily 0x7C00/16 = 0x7C0
;So, jump to a known address: CS=0x0000 and IP=0x7C00 and set up a new stack immediately
jmp  0x0000:main

%include "../asm_helpers/print.asm"

; PRINT 3 is a macro that prints given a string, text color, and background color
%macro PRINTI 1  ; print an info line (i.e. PRINT1, but without having to specify a dark grey color)
	mov   esi, %1
	call  print_info
%endmacro
%macro FAIL 2  ; a failure condition--print an error and hang
	mov  esi, %2
	%1   print_fail_hang
%endmacro

print_info:
	SET_COLOR_L(0x8,0x0)
	call  print_string
	ret
print_fail_hang:
	SET_COLOR_L(0x4,0x0)
	call  print_string
	jmp   $

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

	;	Load GDT thing
	;cli              ; we don't want to be interrupted right now
	xor     ax, ax
	mov     ds, ax      ; clear ds - this register is used by lgdt
	lgdt    [gdt_desc]  ; Load the GDT descriptor
	PRINTI  str_lk_5

	;	Enable video mode 0x13
	PRINTI  str_lk_6
	mov     ah, 0x00
	mov     al, 0x13
	int     0x10

	;	Enable protected mode by setting bit 0 of CR0.
	mov  eax, cr0
	or   eax, 1
	mov  cr0, eax

	;	Long jump to clear prefetch queue and enter kernel
	jmp SEGMENT_ID_CODE:clear_queue_and_jmp

[BITS 32]
;By far jumping we clear the prefetch queue, apparently.
clear_queue_and_jmp:
	; The first order of business is to set up the registers properly
	; Set up the stack and data segments, and point at the screen buffer

	; According to the GDT we defined, the Code Segment descriptor is 8 (0x08)
	; and the data/stack segment descriptor is 16 (0x10). Point all segment 
	; registers except for cs at the data segment.

	mov  ax, SEGMENT_ID_DATA
	mov  ds, ax
	mov  es, ax
	mov  fs, ax
	mov  gs, ax
	mov  ss, ax

	; Point the stack pointer at the stack
	mov  esp, ADDRESS_STACK

	;mov  eax, 0x000b8000
	;mov  ebx, 0x07690748
	;mov  [eax], ebx

	;Jump to the kernel!
	jmp SEGMENT_ID_CODE:ADDRESS_KERNEL

DAP:  ; Disk Address Packet
	;Size of DAP
	db  0x10
	;Unused
	db  0
	;Number of sectors to read
	dw  0x35     ; int 13 resets this to # of blocks actually read/written
	;Offset and segment of destination
	;db  0x00    ; offset
	;db  0x7C
	;db  0x00    ; segment
	;db  0x00
	dw  ADDRESS_KERNEL  ; offset
	dw  0x0000          ; segment
	;LBA of start
	dd  1       ; lower 4 bytes
	dd  0       ; upper 4 bytes

bootdisk  db 0

;ASCII 13 is \r, ASCII 10 is \n
str_ss_1  db "MOSS 0.3 Bootloader",13,10,"Ian Mallett",13,10,0

str_lk_2  db "INT13 supported!",13,10,0
str_lk_3  db "load start:",13,10,0
str_lk_4  db "load done!",13,10,0
str_lk_5  db "GDT loaded!",13,10,0
str_lk_6  db "enabling video mode 0x13",0

str_f_a20    db "no A20!",0
str_f_int13  db "no INT13!",0
str_f_load   db "load failed!",0


gdt:       ; Address for the GDT
gdt_null:  ; Null Segment
	dd 0
	dd 0
gdt_code:  ; Code segment, read/execute, nonconforming
	dw 0FFFFh
	dw 0
	db 0
	db 10011010b
	db 11001111b
	db 0
gdt_data:  ; Data segment, read/write, expand down
	dw 0FFFFh
	dw 0
	db 0
	db 10010010b
	db 11001111b
	db 0
gdt_end:

gdt_desc:                 ; The GDT descriptor
	dw gdt_end - gdt - 1  ; Limit (size)
	dd gdt                ; Address of the GDT

;Fill the rest of sector with 0
TIMES  510 - ($ - $$) db 0

;Add boot signature at the end of bootloader
DW 0xAA55