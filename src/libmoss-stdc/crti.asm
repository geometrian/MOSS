;https://wiki.osdev.org/Creating_a_C_Library#crtbegin.o.2C_crtend.o.2C_crti.o.2C_and_crtn.o

;section .init
;global _init
;_init:
;	push   rbp
;	mov    rbp, rsp
;	; Compiler puts contents of "crtbegin.o" .init section here
;
;section .fini
;global _fini
;_fini:
;	push   rbp
;	mov    rbp, rsp
;	; Compiler puts contents of "crtbegin.o" .fini section here
