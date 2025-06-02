section .text



;bits 16
;
;
;
;bits 32
;
;global _core_lgdt32
;
;_core_lgdt32:
;
;	; Load GDT
;	lgdt   [esp + 4]
;
;	; However, this has no effect until new segment selectors are loaded into the segment registers.
;	; We can't set `cs` directly, however; a far jump (of the form `CS:IP`) sets it.  Here, the new
;	; segment selector is `0x80`, which is `0b0000000000001'0'00`---i.e., index=1 in the GDT with
;	; privilege 0, i.e., the code-segment descriptor for the kernel.
;	jmp    0x0008:_core_lgdt32_set_data_seg_sels
;
;_core_lgdt32_set_data_seg_sels:
;	; Set the other registers `0x10`, which is `0b0000000000010'0'00`---i.e., index=2 in the GDT
;	; with privilege 0, i.e., the data-segment descriptor for the kernel.
;	mov   ax, 0x0010
;	mov   ds, ax
;	mov   es, ax
;	mov   fs, ax
;	mov   gs, ax
;	mov   ss, ax
;
;	ret



bits 64

global _core_lgdt64

_core_lgdt64:

	lgdt   [rdi]

	; Same idea but with far return
	; https://wiki.osdev.org/GDT_Tutorial#Long_Mode_2
	push   0x0008
	lea    rax, [rel _core_lgdt64_set_data_seg_sels]
	push   rax
	retfq

_core_lgdt64_set_data_seg_sels:
	mov   ax, 0x0010
	mov   ds, ax
	mov   es, ax
	mov   fs, ax
	mov   gs, ax
	mov   ss, ax

	ret



;; Load GDT (note interrupts must be off)
;;
;; This would be easy enough to do with inline assembly, but clang has an apparent bug where it
;; flatly does not understand far-jump syntax when compiling assembly inline.  In particular:
;;
;;	jmp   0x08:set_data_segs
;;
;; —does not work.  There is a variant syntax that clang doesn't like either:
;;
;;	jmp   0x08, set_data_segs
;;
;; Nor do hacks with trying to set AT&T syntax work.  GCC does it fine.  Albeit, perhaps the problem
;; was that this is only valid in 16-/32-bit mode; in 64-bit we need something else (see below).
;;
;; Anyway, we're in a ".asm" file instead of in C++.  This one is set up for nasm.
;
;section .text
;
;global _gdt_setup
;
;_gdt_setup:
;	%ifidn __OUTPUT_FORMAT__, win32
;
;		
;
;	%elifidn __OUTPUT_FORMAT__, win64
;
;		
;
;	%else
;
;		%error "Unknown architecture!"
;
;	%endif
;
;	_gdt_setup_set_data_segs:
;	
;
;; See also:
;;	• Great explanation https://stackoverflow.com/a/23979175/
;;	• "Segment Selector" in notes
;;	• https://wiki.osdev.org/Segment_Selector
;;	• Far jump syntax: https://stackoverflow.com/a/65351607/
;;	• https://wiki.osdev.org/GDT_Tutorial#Long_Mode_2
;;	• Calling conventions: https://en.wikipedia.org/wiki/X86_calling_conventions#List_of_x86_calling_conventions
;;	• https://www.felixcloutier.com/x86/lgdt:lidt
