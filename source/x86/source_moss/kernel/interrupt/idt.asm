idt_ptr:
	dw  0 ;For limit storage
	dd  0 ;For base storage

global idt_lidt
idt_lidt:
	mov  eax, [esp + 4]
	mov  [idt_ptr + 2], eax
	mov  ax, [esp + 8]
	mov  [idt_ptr], ax

	lidt  [idt_ptr]

	ret
