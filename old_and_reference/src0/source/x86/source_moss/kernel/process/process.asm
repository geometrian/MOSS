;http://wiki.osdev.org/Kernel_Multitasking#switch.S

global _load_process
_load_process:
	;;Get pointer to registers passed in
	;mov eax, [ebp+8]
	;
	;;Set return address
	;push eax
	;mov eax, [eax+32] ;eax = eip
	;mov [ebp+4], eax
	;pop eax
	;
	;;Load general-purpose registers
	;mov edi, [eax+20]
	;mov esi, [eax+16]
	;mov edx, [eax+12]
	;mov ecx, [eax+ 8]
	;mov ebx, [eax+ 4]
	;mov eax, [eax+ 0] ;Note comes last
	;
	;;xchg  bx, bx   ;Bochs magic breakpoint
	;
	;ret

	;Get pointers to register sets passed in
	mov eax, [ebp+12] ;Argument 0 (old stack)
	;	[eax+76] ss
	;	[eax+72] useresp
	;	[eax+68] eflags
	;	[eax+64] cs
	;	[eax+60] eip
	;	[eax+56] error code or 0
	;	[eax+52] interrupt index
	;	[eax+48] debug marker
	;	             <--              |
	;	[eax+44] eax    \             |
	;	[eax+40] ecx    |           lower
	;	[eax+36] edx    |         addresses;
	;	[eax+32] ebx    /           stack
	;	[eax+28] esp ---            growth
	;	[eax+24] ebp                  |
	;	[eax+20] esi                  |
	;	[eax+16] edi                  v
	;	[eax+12] ds
	;	[eax+ 8] es
	;	[eax+ 4] fs
	;	[eax+ 0] gs
	mov ebx, [ebp+ 8] ;Argument 1 (new stack)
	;	[ebx+32] eip
	;	[ebx+28] ebp
	;	[ebx+24] esp
	;	[ebx+20] edi
	;	[ebx+16] esi
	;	[ebx+12] edx
	;	[ebx+ 8] ecx
	;	[ebx+ 4] ebx
	;	[ebx+ 0] eax

	;Corrupt this state so that when the interrupt handler returns, we return
	;	to a different process!

	xchg  bx, bx   ;Bochs magic breakpoint

	;	ebp
	mov ecx, [ebx+28]
	mov [eax+24], ecx
	;	esp
	mov ecx, [ebx+24]
	mov [eax+28], ecx
	;	edi
	mov ecx, [ebx+20]
	mov [eax+16], ecx
	;	esi
	mov ecx, [ebx+16]
	mov [eax+20], ecx

	;	edx
	mov ecx, [ebx+12]
	mov [eax+36], ecx
	;	ecx
	mov ecx, [ebx+ 8]
	mov [eax+40], ecx
	;	ebx
	mov ecx, [ebx+ 4]
	mov [eax+32], ecx
	;	eax
	mov ecx, [ebx+ 0]
	mov [eax+44], ecx


	;Return.  When we get back to the interrupt handler, we'll "iret" "back"
	;	into a different process!
	ret

global switch_process
switch_process:
	;pusha
	;pushf
	;
	;;Push CR3
	;mov eax cr3
	;push eax
	;
	;mov 44(%esp), %eax ;The first argument, where to save
	;mov %ebx, 4(%eax)
	;mov %ecx, 8(%eax)
	;mov %edx, 12(%eax)
	;mov %esi, 16(%eax)
	;mov %edi, 20(%eax)
	;mov 36(%esp), %ebx ;EAX
	;mov 40(%esp), %ecx ;IP
	;mov 20(%esp), %edx ;ESP
	;add $4, %edx ;Remove the return value ;)
	;mov 16(%esp), %esi ;EBP
	;mov 4(%esp), %edi ;EFLAGS
	;mov %ebx, (%eax)
	;mov %edx, 24(%eax)
	;mov %esi, 28(%eax)
	;mov %ecx, 32(%eax)
	;mov %edi, 36(%eax)
	;pop %ebx ;CR3
	;mov %ebx, 40(%eax)
	;push %ebx ;Goodbye again ;)
	;mov 48(%esp), %eax ;Now it is the new object
	;mov 4(%eax), %ebx ;EBX
	;mov 8(%eax), %ecx ;ECX
	;mov 12(%eax), %edx ;EDX
	;mov 16(%eax), %esi ;ESI
	;mov 20(%eax), %edi ;EDI
	;mov 28(%eax), %ebp ;EBP
	;push %eax
	;mov 36(%eax), %eax ;EFLAGS
	;push %eax
	;popf
	;pop %eax
	;mov 24(%eax), %esp ;ESP
	;push %eax
	;mov 40(%eax), %eax ;CR3
	;mov %eax, %cr3
	;pop %eax
	;push %eax
	;mov 32(%eax), %eax ;EIP
	;xchg (%esp), %eax ;We do not have any more registers to use as tmp storage
	;mov (%eax), %eax ;EAX
	;
	;;This ends all!
	;ret
