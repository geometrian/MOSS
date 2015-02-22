;Prints a message informing that it is the bootloader level 1
;	Modifies DI,SI
print_level1:
	PRINT  str_l1_1, 0x7, 0x0
	ret

	str_l1_1 db "Bootloader level 1: ",0