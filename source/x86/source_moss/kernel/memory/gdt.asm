gdt_ptr:
	dw  0 ;For limit storage
	dd  0 ;For base storage

global gdt_lgdt
gdt_lgdt:
	mov  eax, [esp + 4]
	mov  [gdt_ptr + 2], eax
	mov  ax, [esp + 8]
	mov  [gdt_ptr], ax

	lgdt  [gdt_ptr]

	ret

;See http://wiki.osdev.org/Segmentation
;The 0x08 is the offset of the kernel code selector in the GDT (the second 8-byte entry, following the null-entry)
;The 0x10 is the offset of the kernel data selector in the GDT (the third 8-byte entry, following the code selector at 0x08)
global reload_segments
reload_segments:
	;Reload CS register containing code selector
	jmp  0x08:reload_CS
reload_CS:
	;Reload data segment registers:
	mov  ax, 0x10
	mov  ds, ax
	mov  es, ax
	mov  fs, ax
	mov  gs, ax
	mov  ss, ax

	ret
