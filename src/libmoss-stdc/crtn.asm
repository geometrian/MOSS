;https://wiki.osdev.org/Creating_a_C_Library#crtbegin.o.2C_crtend.o.2C_crti.o.2C_and_crtn.o

;section .init
;	; Compiler puts contents of "crtend.o" .init section here
;	pop   rbp
;	ret
;
;section .fini
;	; Compiler puts contents of "crtend.o" .fini section here
;	pop   rbp
;	ret
