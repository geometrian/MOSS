#pragma once

namespace KEYBOARD {


/* "bucky bits"
0x0100 is reserved for non-ASCII keys, so start with 0x200 */
#define	KBD_META_ALT	0x0200	/* Alt is pressed */
#define	KBD_META_CTRL	0x0400	/* Ctrl is pressed */
#define	KBD_META_SHIFT	0x0800	/* Shift is pressed */
#define	KBD_META_ANY	(KBD_META_ALT | KBD_META_CTRL | KBD_META_SHIFT)
#define	KBD_META_CAPS	0x1000	/* CapsLock is on */
#define	KBD_META_NUM	0x2000	/* NumLock is on */
#define	KBD_META_SCRL	0x4000	/* ScrollLock is on */

/* "ASCII" values for non-ASCII keys. All of these are user-defined.

Hrrm. Unicode defines code pages for pseudographics (e.g. box-drawing
characters). I wonder it defines anything for keys like these?

function keys: */
#define	KEY_F1		0x80
#define	KEY_F2		(KEY_F1 + 1)
#define	KEY_F3		(KEY_F2 + 1)
#define	KEY_F4		(KEY_F3 + 1)
#define	KEY_F5		(KEY_F4 + 1)
#define	KEY_F6		(KEY_F5 + 1)
#define	KEY_F7		(KEY_F6 + 1)
#define	KEY_F8		(KEY_F7 + 1)
#define	KEY_F9		(KEY_F8 + 1)
#define	KEY_F10		(KEY_F9 + 1)
#define	KEY_F11		(KEY_F10 + 1)
#define	KEY_F12		(KEY_F11 + 1)
/* cursor keys */
#define	KEY_INS		0x90
#define	KEY_DEL		(KEY_INS + 1)
#define	KEY_HOME	(KEY_DEL + 1)
#define	KEY_END		(KEY_HOME + 1)
#define	KEY_PGUP	(KEY_END + 1)
#define	KEY_PGDN	(KEY_PGUP + 1)
#define	KEY_LFT		(KEY_PGDN + 1)
#define	KEY_UP		(KEY_LFT + 1)
#define	KEY_DN		(KEY_UP + 1)
#define	KEY_RT		(KEY_DN + 1)
/* print screen/sys rq and pause/break */
#define	KEY_PRNT	(KEY_RT + 1)
#define	KEY_PAUSE	(KEY_PRNT + 1)
/* these return a value but they could also act as additional bucky keys */
#define	KEY_LWIN	(KEY_PAUSE + 1)
#define	KEY_RWIN	(KEY_LWIN + 1)
#define	KEY_MENU	(KEY_RWIN + 1)


typedef struct {
	unsigned access_byte, eip;
} vector;

//http://inportb.com/inportb/
inline unsigned char inportb(unsigned int port) {
	//read a byte from a port
	unsigned char ret;
	asm(
		"inb %%dx,%%al":"=a"(ret):"d"(port)
	);
	return ret;
}
inline void outportb(unsigned int port,unsigned char value) {
	//write a byte to a port
	asm(
		"outb %%al,%%dx": :"d"(port),"a"(value)
	);
}

static unsigned char convert(unsigned char code) {
	static const unsigned char set1_map[] =
	{
		/* 00 */0,0x1B,'1','2','3','4','5','6',
		/* 08 */'7','8','9','0','-','=','\b','\t',
		/* 10 */'q','w','e','r','t','y','u','i',
		/* 1Dh is left Ctrl */
		/* 18 */'o','p','[',']','\n',0,'a','s',
		/* 20 */'d','f','g','h','j','k','l',';',
		/* 2Ah is left Shift */
		/* 28 */'\'','`',0,'\\','z','x','c','v',
		/* 36h is right Shift */
		/* 30 */'b','n','m',',','.','/',0,0,
		/* 38h is left Alt, 3Ah is Caps Lock */
		/* 38 */0,' ',0,KEY_F1,KEY_F2,KEY_F3,KEY_F4,KEY_F5,
		/* 45h is Num Lock, 46h is Scroll Lock */
		/* 40 */KEY_F6,KEY_F7,KEY_F8,KEY_F9,KEY_F10,0,0,KEY_HOME,
		/* 48 */KEY_UP,KEY_PGUP,'-',KEY_LFT,'5',KEY_RT,'+',KEY_END,
		/* 50 */KEY_DN,KEY_PGDN,KEY_INS,KEY_DEL,0,0,0,KEY_F11,
		/* 58 */KEY_F12
	};

	//ignore set 1 break codes
	if (code >= 0x80) return 0;
	//ignore invalid scan codes
	if (code >= sizeof(set1_map) / sizeof(set1_map[0])) return 0;
	//convert raw scancode in code to unshifted ASCII
	return set1_map[code];
}

/*static void keyboard_irq(void) {
	unsigned char scan_code, temp;

	//You MUST read port 60h to clear the keyboard interrupt
	scan_code = inportb(0x60);
	temp = convert(scan_code);
	//if (temp != 0) putch(temp);
	if (temp != 0) {
		CONSOLE::Console::draw(5,8,"HOORAY: ");
		CONSOLE::Console::draw(5,9,temp);
	}

	//reset hardware interrupt at 8259 chip
	outportb(0x20, 0x20);
}*/

/*****************************************************************************
The BIOS routes IRQs 0-7 to INTs 8-15, which are CPU-reserved interrupts
in protected mode. This makes it difficult to determine if, say, interrupt 8
was Double Fault or IRQ 0. (The BIOS routes IRQs 8-15 to INTs 112-119)

The 8259 data sheet is hard to understand.
If you don't want to wade through it, just copy this code.
*****************************************************************************/
/*static void init_8259s(void) {
	static const unsigned irq0_int = 0x20, irq8_int = 0x28;

	//Initialization Control Word #1 (ICW1)
	outportb(0x20, 0x11);
	outportb(0xA0, 0x11);
	//ICW2:
	//	route IRQs 0-7 to INTs 20h-27h
	outportb(0x21, irq0_int);
	//	route IRQs 8-15 to INTs 28h-2Fh
	outportb(0xA1, irq8_int);

	//ICW3
	outportb(0x21, 0x04);
	outportb(0xA1, 0x02);
	//ICW4
	outportb(0x21, 0x01);
	outportb(0xA1, 0x01);

	//enable IRQ0 (timer) and IRQ1 (keyboard)
	outportb(0x21, ~0x03);
	outportb(0xA1, ~0x00);
}

void setup(void) {
	KEYBOARD::init_8259s();
}*/

}