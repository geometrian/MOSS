;Prints a message informing that it is the bootloader level 2
;	Modifies DI,SI
print_level2:
	PRINT  str_l2_1, 0x7, 0x0
	ret

	str_l2_1 db "Bootloader level 2: ",0