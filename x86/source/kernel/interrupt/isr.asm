[EXTERN isr_handler]

[GLOBAL isr]

isr:
	pusha
	push  gs
	push  fs
	push  ds
	push  es

	call  isr_handler

	pop  es
	pop  ds
	pop  fs
	pop  gs
	popa
	iret