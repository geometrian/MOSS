[GLOBAL gdt_lgdt]

gdt_ptr:
	dw 0 ;For limit storage
	dd 0 ;For base storage

gdt_lgdt:
	mov  eax, [esp + 4]
	mov  [gdt_ptr + 2], eax
	mov  ax, [esp + 8]
	mov  [gdt_ptr], ax

	lgdt  [gdt_ptr]

	ret


[GLOBAL idt_lidt]

idt_ptr:
	dw 0 ;For limit storage
	dd 0 ;For base storage

idt_lidt:
	mov  eax, [esp + 4]
	mov  [idt_ptr + 2], eax
	mov  ax, [esp + 8]
	mov  [idt_ptr], ax

	lidt  [idt_ptr]

	ret


[GLOBAL reload_segments]

;See http://wiki.osdev.org/Segmentation
;The 0x08 is the offset of the kernel code selector in the GDT (the second 8-byte entry, following the null-entry)
;The 0x10 is the offset of the kernel data selector in the GDT (the third 8-byte entry, following the code selector at 0x08)
reload_segments:
	; Reload CS register containing code selector:
	jmp  0x08:reload_CS
reload_CS:
	; Reload data segment registers:
	mov  ax, 0x10
	mov  ds, ax
	mov  es, ax
	mov  fs, ax
	mov  gs, ax
	mov  ss, ax

	ret



[section .text]

[GLOBAL load_idt]

load_idt:
	;load the IDT, this requires that the data section with
	;the IDT exists
	lidt  [idt_pointer]

	ret

[section .data]
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; the IDT with it's descriptors
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
start_of_idt:
	;interrupt 0
	dw 0x0000
	dw 0x10
	dw 0x8E00
	dw 0x20

	;interrupt 1
	dw 0x0000
	dw 0x10
	dw 0x8E00
	dw 0x20

	;interrupt 2, intel reserved, we set the 'present' bit to 0 on this one
	dw 0x0000
	dw 0x10
	dw 0xE00
	dw 0x20

	;interrupts 3-14 now, since we are making the descriptors
	;identical, we are going to loop to get them all(12 total)
	%rep 0xC
		dw 0x0000
		dw 0x10
		dw 0x8E00
		dw 0x20
	%endrep

	;interrupt 15, intel reserved, we set the 'present' bit to 0 on this one
	dw 0x0000
	dw 0x10
	dw 0xE00
	dw 0x20

	;interrupt 16
	dw 0x0000
	dw 0x10
	dw 0x8E00
	dw 0x20
end_of_idt:

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; now for the IDT pointer
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
idt_pointer:
	dw end_of_idt - start_of_idt - 1
	dd start_of_idt