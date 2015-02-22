[BITS 32]

[global jump]
[extern kernel_main]

jump:
	;mov  eax, 0x000b8000
	;mov  ebx, 0x07690748
	;mov  [eax], ebx

	;jmp  $
	call  kernel_main