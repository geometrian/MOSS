void kernel_main(void) {
	//Print "K" to the screen at position 0,0
	unsigned short *video = (unsigned short *)0XB8000;
	video[0] = 75;
	video[1] = 0x07;

	HANG: goto HANG;
}