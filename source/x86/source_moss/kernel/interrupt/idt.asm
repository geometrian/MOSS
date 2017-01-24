_idt_ptr:
	dw  0 ;For limit storage
	dd  0 ;For base storage

global _lidt
_lidt:
	mov  eax, [esp + 4]
	mov  [_idt_ptr + 2], eax
	mov  ax, [esp + 8]
	mov  [_idt_ptr], ax

	lidt  [_idt_ptr]

	ret
