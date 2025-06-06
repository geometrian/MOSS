;The ISRs are defined in this file.

;Note: this file's code is heavily based on http://www.osdever.net/bkerndev/Docs/isrs.htm.  Differences are noted.  See also
;	http://www.acm.uiuc.edu/sigops/roll_your_own/i386/idt.html

;Ideally, we'd have a separate ISR entry point for each interrupt vector, each calling the corresponding C handler.  However, this
;	is complicated by the fact that some interrupts push error codes and some do not.  The error code, if it exists, must be popped
;	off the stack before the "iret" instruction (see http://stackoverflow.com/questions/491613/return-from-interrupts-in-x86).  This
;	makes it very difficult if not impossible to call the major section of the ISR (that sets up information and calls the handler)
;	in a separate routine.  One could duplicate all that code, but there are 256 ISRs to make.  It seems like a waste, even if it
;	might be more direct.

;Some sources (e.g. the link above) recommend pushing a dummy byte in the cases where the CPU does not do so automatically itself,
;	thus simplifying all the ISRs enough to call a common routine, which calls a common C++ handler (which then delegates the interrupt
;	to another, appropriate C++ specific handler) and then finishes up.  This mechanism is adopted here since it is ultimately the
;	cleanest.

;Each ISR:
;	--Pushes a dummy error code iff a genuine one was not.
;	--Jumps to the common subroutine "isr_delegator_asm"
;The subroutine "isr_delegator_asm":
;	--Pushes the interrupt number (ultimately for the common C++ handler's benefit)
;	--Pushes some key information, including the general purpose registers and the segment registers.  The purpose of the latter
;	  seems to be to allow the C++ specific interrupt handler to know whether the originator came from the kernel or userland?
;	--Calls the C++ common handler.
;	--When the handler returns, the information that was pushed is popped.
;	--Returns from interrupt vector using special "iret" instruction.

;Note: not enabling/disabling hardware interrupts with "sti"/"cli" is intentional.  See the notes for this section.

;TODO: remove debugging markers (and in "types.hpp"'s "InterruptState")


;ISR that DOES NOT pass its own error code
%macro ISR_NOERROR 1
	global isr%1_asm
	extern isr%1
	isr%1_asm:
		;xchg  bx, bx   ;Bochs magic breakpoint
		push  dword  0 ;dummy error code
		push  dword %1 ;interrupt index
		push  dword 0xABCD1234 ;debug marker
		;xchg  bx, bx   ;Bochs magic breakpoint
		jmp   isr_delegator_asm
%endmacro
;ISR that DOES pass its own error code
%macro ISR_ERROR 1
	global isr%1_asm
	extern isr%1
	isr%1_asm:
		               ;error code was pushed here automatically by the CPU prior to entry
		;xchg  bx, bx   ;Bochs magic breakpoint
		push  dword %1 ;interrupt index
		push  dword 0xDEADBEEF ;debug marker
		;xchg  bx, bx   ;Bochs magic breakpoint
		jmp   isr_delegator_asm
%endmacro

isr_delegator_asm:
	pusha
	push   ds
	push   es
	push   fs
	push   gs

	;Load the Kernel Data Segment descriptor!
	mov  ax, 0x10
	mov  ds, ax
	mov  es, ax
	mov  fs, ax
	mov  gs, ax
	mov  eax, esp  ;Push us the stack

	push  eax

	;The link above that all this is heavily based on claims that this is a special instruction that doesn't change "eip".  Since "call" is equivalent
	;	to "push ret_addr\njmp func" (http://stackoverflow.com/questions/7060970/substitutes-for-x86-assembly-call-instruction), and "jmp" ought to change
	;	"eip", I don't see how that's possible.  Granted, the *net* effect of this instruction is nothing on "eip"?

	;However, the more likely reason is that the call must use absolute addressing instead of eip-relative addressing.  That also explains why the
	;	calling procedure is two instructions instead of the obvious "call  isr%1".  See also http://forum.osdev.org/viewtopic.php?f=1&t=11304.
	extern isr_delegator_cpp
	mov   eax, isr_delegator_cpp
	call  eax

	pop  eax

	pop   gs
	pop   fs
	pop   es
	pop   ds
	popa

	;Clean up the pushed ISR number and the pushed error code (whether we pushed it xor the CPU pushed it automatically).
	add  esp, 8

	;Clean up debugging markers
	add  esp, 4

	;TODO: http://stackoverflow.com/questions/11756153/whats-the-difference-between-iret-and-iretd-iretq?
	iret ;Special return instruction (pops 5 things at once: cs, eip, eflags, ss, and esp)

;Define all 256 interrupt vectors
;	ISRs [0,19] are well-defined
ISR_NOERROR  0
ISR_NOERROR  1
ISR_NOERROR  2
ISR_NOERROR  3
ISR_NOERROR  4
ISR_NOERROR  5
ISR_NOERROR  6
ISR_NOERROR  7
ISR_ERROR    8
ISR_NOERROR  9
ISR_ERROR   10
ISR_ERROR   11
ISR_ERROR   12
ISR_ERROR   13
ISR_ERROR   14
ISR_NOERROR 15
ISR_NOERROR 16
ISR_NOERROR 17
ISR_NOERROR 18
ISR_NOERROR 19
;	ISRs [20,31] are Intel-reserved (and are currently considered fatal; see "isrs.hpp")
%assign i 20
%rep    12
	ISR_NOERROR i
	%assign i i+1
%endrep
;	ISRs [32,255] are available for OS usage.
;	ISRs [32,47] are the remapped 16 IRQs from the PIC.  These say they don't pass error codes: http://www.osdever.net/bkerndev/Docs/irqs.htm, http://forum.osdev.org/viewtopic.php?f=1&t=26875&p=224971#p224971
%assign i 32
%rep    16
	ISR_NOERROR i
	%assign i i+1
%endrep
;	ISRs [48,255] are unallocated.  These says that "int" instructions don't pass error codes: http://www.google.com/url?sa=t&rct=j&q=&esrc=s&source=web&cd=3&cad=rja&ved=0CD0QFjAC&url=http%3A%2F%2Fopensecuritytraining.info%2FIntermediateX86_files%2FIntermediateIntelx86-Part3.ppt&ei=lY7WUdLOI7GxigK6lID4Ag&usg=AFQjCNGIi5fJU-BnAZkqhBHrl-VJTWVsdA&sig2=rlDZz39OUDRKGhAs_w-BdA&bvm=bv.48705608,d.cGE
%assign i 48
%rep    224
	ISR_NOERROR i
	%assign i i+1
%endrep
