; https://wiki.osdev.org/Creating_a_C_Library#crt0.o

section .text

;extern _init
;extern __libc_init_array
;extern main
;extern exit
;
;global _start
;_start:
;	; Set up end of the stack frame linked list.
;	mov    rbp, $0
;	push   rbp
;	push   rbp
;	mov    rbp, rsp
;
;	; We need those in a moment when we call main.
;	push   rsi
;	push   rdi
;
;	; Prepare signals, memory allocation, stdio and such.
;	; call   initialize_standard_library
;
;	; Run the global constructors.
;	call   _init
;	call __libc_init_array
;
;	; Restore argc and argv.
;	pop   rdi
;	pop   rsi
;
;	; Run main
;	call   main
;
;	; Terminate the process with the exit code.
;	mov    edi, eax
;	call   exit   ; note calls `_fini(⋯)` too

;global vfprintf
;vfprintf:
;	push   rbp
;	mov    rbp,rsp
;	sub    rsp,0x40
;	mov    [rbp-0x8],rdi
;	mov    [rbp-0x10],rsi
;	mov    [rbp-0x18],rdx
;
;	lea    rax,[rbp-0x30]
;	mov    rcx,[rbp-0x18]
;	mov    rdx,[rcx+0x10]
;	mov    [rax+0x10],rdx
;	;vmovdqu xmm0,[rcx]
;	;vmovdqu [rax],xmm0
;
;	mov    dword [rbp-0x34],0x0
;	lea    rax,[rbp-0x30]
;	mov    eax,[rbp-0x34]
;	add    rsp,0x40
;	pop    rbp
;	ret
