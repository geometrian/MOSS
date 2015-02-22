#include "io.h"

#include "../../includes.h"


namespace MOSS { namespace IO {


template <> uint8_t recv(unsigned short port) {
	uint8_t value;
	asm volatile(
		"inb  %1, %0"
		:"=a"(value)
		:"Nd"(port)
	);
	return value;
}
template <> uint16_t recv(unsigned short port) {
	uint16_t value;
	asm volatile(
		"inw  %1, %0"
		:"=a"(value)
		:"Nd"(port)
	);
	return value;
}
template <> uint32_t recv(unsigned short port) {
	uint32_t value;
	asm volatile(
		"inl  %1, %0"
		:"=a"(value)
		:"Nd"(port)
	);
	return value;
}
template <> void send(unsigned short port, uint8_t value) {
	asm volatile(
		"outb  %0, %1"
		:
		:"a"(value),
		"Nd"(port)
	);
}
template <> void send(unsigned short port, uint16_t value) {
	asm volatile(
		"outw  %0, %1"
		:
		:"a"(value),
		"Nd"(port)
	);
}
template <> void send(unsigned short port, uint32_t value) {
	asm volatile(
		"outl  %0, %1"
		:
		:"a"(value),
		"Nd"(port)
	);
}

/*void wait(void) {
	asm volatile(
		"jmp 1f"
		"1:jmp 2f"
		"2:"
	);
}*/
void wait(void) {
	//port 0x80 is used for "checkpoints" during POST.
	//The Linux kernel seems to think it is free for use :-/
	asm volatile(
		"outb %%al, $0x80"
		:
		:"a"(0)
	);
}


}}