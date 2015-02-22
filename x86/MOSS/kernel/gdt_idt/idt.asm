[GLOBAL idt_lidt]

idt_lidt:
	mov   eax, [esp+4]  ;Get the pointer to the IDT, passed as a parameter.
	lidt  [eax]         ;Load the IDT pointer.

	ret