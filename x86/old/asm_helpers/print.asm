%ifndef __FILE_PRINT__
%define __FILE_PRINT__

;STR, STYLE; STYLE is background, foreground
%macro PRINTSTR 2
	mov   bl, %2
	mov   si, %1
	call  print_string
%endmacro
%macro PRINTSTR1 2
	call  print_level1
	mov   bl, %2
	mov   si, %1
	call  print_string
%endmacro

;print_system_specs:
;	int  12h    ;Get RAM size (in KBs)
;	shl  ax, 6
;	mov  cx, ax
;	call print_short

;	mov   bl, 0x06      ;background, foreground
;	mov   si, str_l_1
;	call  print_string

;	ret

;mov ah, 9           ; Print "===="
;mov al, '='         ;
;mov bx, 7           ;
;mov cx, 4           ;
;int 10h             ;
print_level1:
	PRINTSTR str_level1
	ret

	str_level1 db "Bootloader level 1: "
print_string:
	;String pointer is in register SI

	mov  ah, 0x0E  ;teletype
	mov  bh, 0x00  ;page number

	print_string_next_character:
		mov   al, [si]
		inc   si

		cmp   al, 0x00
		je    print_string_exit

		int   0x10

		jmp   print_string_next_character
	print_string_exit:
		ret

;print_short:
	;Short to print is in c
;print_byte:
	;Byte to print is in cl
	;Destructive; ruins cl's value
;	print_byte_digit:
		

;		cmp  ax, 0xA
;		jl   print_byte_digit
;	ret

%endif