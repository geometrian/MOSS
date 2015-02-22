;See http://www.osdever.net/bkerndev/Docs/isrs.htm

;Ideally, we'd have a separate ISR entry point for each interrupt vector, each calling the corresponding C handler.  However, this
;is complicated by the fact that some interrupts push error codes and some do not.  The error code, if it exists, must be popped
;off the stack before the "iret" instruction (see http://stackoverflow.com/questions/491613/return-from-interrupts-in-x86).  This
;makes it very difficult if not impossible to call the major section of the ISR (that sets up information and calls the handler)
;in a separate routine.  One could duplicate all that code, bute there are 256 ISRs to make.  It seems like a waste, even if it
;might be more direct.

;Some sources (e.g. the link above) recommend pushing a dummy byte in the cases where is does not, thus simplifying all the ISRs
;enough to call a common routine, which calls a common C++ handler (which then delegates the interrupt to another, appropriate
;C++ handler) and then finishes up.  This mechanism is adopted here since it is ultimatly the cleanest.

;Each ISR:
;	--Disables interrupts
;	--Pushes a dummy error code if a real one was not.
;	--Pushes the interrupt number (for the subroutine's benefit)
;	--Pushes some key information, including the registers
;	--Calls its handler
;	--When the handler returns, interrupts are reenabled.
;	--Returns from interrupt vector using special "iret" instruction.



;ISR that does NOT pass its own error code (pass a dummy 
%macro ISR_NOERROR 1
	global isr%1_asm
	extern isr%1
	isr%1_asm:
		cli  ;Disable interrupts

		push  byte 0 ;dummy code (sometimes)
		push  byte 1 ;interrupt number

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
		mov  eax, esp   ; Push us the stack

		push  eax

		mov   eax, isr%1
		call  eax

		pop  eax

		pop   gs
		pop   fs
		pop   es
		pop   ds
		popa

		add  esp, 8  ; Clean up the pushed error code and pushed ISR number (addresses are 4-byte aligned, so two bytes took up eight on the stack)

		sti ;Reenable interrupts?

		iret ;Special return instruction (pops 5 things at once: cs, eip, eflags, ss, and esp)
%endmacro

;Define all 256 interrupt vectors TODO: actually fewer
%assign i 0
%rep    20
	ISR i

	%assign i i+1
%endrep






	cli

	

	