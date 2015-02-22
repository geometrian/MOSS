.global _start
_start:
	mov sp,#0x8000
	b hang
hang:
	b hang
