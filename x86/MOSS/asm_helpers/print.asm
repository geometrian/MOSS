%ifndef __FILE_BL_PRINT__
%define __FILE_BL_PRINT__

;INT 0x10 is a BIOS video interrupt.
;Register al = ASCII value of character to display
;Register ah = 0x0E is Teletype mode (tell BIOS that we want to print one character on screen)
;Register bl = Text Attribute (foreground and background color of character to be displayed)
;Register bh = Page Number (0x00 for most cases)

;0x0 => (  0,  0,  0) => black                ;0x0 => (  0,  0,  0) => black
;0x1 => (  0,  0,168) => dark blue            ;0x8 => ( 87, 87, 87) => dark grey
;0x2 => (  0,168,  0) => dark green           ;0x7 => (168,168,168) => light grey
;0x3 => (  0,168,168) => dark cyan            ;0xF => (255,255,255) => white
;0x4 => (168,  0,  0) => dark red
;0x5 => (168,  0,168) => dark magenta         ;0x4 => (168,  0,  0) => dark red
;0x6 => (168, 87,  0) => dark orange/brown    ;0xC => (255, 87, 87) => light red
;0x7 => (168,168,168) => light grey
;0x8 => ( 87, 87, 87) => dark grey            ;0x6 => (168, 87,  0) => dark orange/brown
;0x9 => ( 87, 87,255) => light blue
;0xA => ( 87,255, 87) => light green          ;0xE => (255,255, 87) => light yellow
;0xB => ( 87,255,255) => light cyan
;0xC => (255, 87, 87) => light red            ;0x2 => (  0,168,  0) => dark green
;0xD => (255, 87,255) => light magenta        ;0xA => ( 87,255, 87) => light green
;0xE => (255,255, 87) => light yellow
;0xF => (255,255,255) => white                ;0x1 => (  0,  0,168) => dark blue
                                              ;0x9 => ( 87, 87,255) => light blue

                                              ;0x3 => (  0,168,168) => dark cyan
                                              ;0xB => ( 87,255,255) => light cyan

                                              ;0x5 => (168,  0,168) => dark magenta
                                              ;0xD => (255, 87,255) => light magenta

%define SET_COLOR_L(TEXT,BG) mov  ah, (BG<<4)|TEXT
%macro PRINT 3
	mov   esi, %1
	SET_COLOR_L(%2,%3)
	call  print_string
%endmacro
%macro CURSOR_SET_X 1
	mov  byte [xpos], %1
%endmacro
%macro CURSOR_SET_Y 1
	mov  byte [ypos], %1
%endmacro
%macro CURSOR_ADD_X 1
	add  byte [xpos], %1
%endmacro
%macro CURSOR_ADD_Y 1
	add  byte [ypos], %1
%endmacro
%macro CURSOR_SET 2
	CURSOR_SET_X %1
	CURSOR_SET_Y %2
%endmacro

;Prints a filled line of color AL on the row ypos
;	Modifies AX(AH,AL),BX(BH,BL), DI,SI
;print_fill_line:
;	mov  bl, al
;	shl  al, 4
;	or   al, bl
;	mov  ah, al
;	mov  al, 33      ; space character

;	mov  bl, [xpos]  ; save xpos

;	xor  bh, bh
;	.loop:
;		call print_character

;		inc  bh
;		CURSOR_ADD_X 1

;		cmp  bh, 80
;		jl   .loop

;	CURSOR_SET_X bl  ; restore xpos
;	ret
;Prints the string in SI starting at (xpos,ypos)
;	Modifies AX(AL), DI,SI, xpos,ypos
print_string:
	.dochar:
		lodsb  ; load string char from SI to AL, and inc SI

		cmp  al, 13
		je   .carriage
		cmp  al, 10
		je   .newline
		cmp  al, 0
		je   .return

		call  print_character

		CURSOR_ADD_X 1

		jmp .dochar
	.carriage:
		CURSOR_SET_X 0
		jmp  .dochar
	.newline:
		CURSOR_ADD_Y 1
		jmp  .dochar
	.return:
		ret
;Prints the color/character in AH/AL at (xpos,ypos)
;	Modifies DI
print_character:
	push   ax
	push   bx

	xor    di, di           ; stosw uses ES:DI to store, so DI=0 is the start of video memory

	movzx  ax, byte [xpos]
	shl    ax, 1            ; times 2 because each position is a color/character of 16 bits
	add    di, ax           ; add x offset

	movzx  ax, byte [ypos]
	mov    bx, 160          ; 2 bytes (char/attrib)
	mul    bx               ; for 80 columns
	add    di, ax           ; add y offset

	pop   bx
	pop   ax

	stosw                   ; write color/character in AX to ES:DI
	ret
;Prints the contents of AX at (xpos,ypos) in hex
;	Modifies DI,SI
;print_reg_ax_hex:
;	push  ax
;	mov   bx, ax

;	SET_COLOR_L(0x0,0xF)

;	mov  dx, 0

;	xor  cl, cl
;	.do_hexit:
;		mov    esi, str_hex
;		mov    dl, bh
;		shr    dl, 4
;		add    si, dx
;		lodsb                   ; load string char from SI to AL, and inc SI
;		call   print_character
;		shl    bx, 4

;		inc  cl
;		CURSOR_ADD_X 1

;		cmp  cl, 4
;		jl   .do_hexit

;	pop  ax
;	ret

;	str_hex  db "0123456789ABCDEF"

xpos  db 0
ypos  db 0

%endif