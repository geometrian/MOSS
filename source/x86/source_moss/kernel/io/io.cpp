#include "io.hpp"


namespace MOSS { namespace IO {


template <>  uint8_t recv(uint16_t port                ) {
	uint8_t value;
	__asm__ __volatile__(
		"inb  %1, %0"
		:"=a"(value)
		:"Nd"(port)
		:
	);
	return value;
}
template <> uint16_t recv(uint16_t port                ) {
	uint16_t value;
	__asm__ __volatile__(
		"inw  %1, %0"
		:"=a"(value)
		:"Nd"(port)
		:
	);
	return value;
}
template <> uint32_t recv(uint16_t port                ) {
	uint32_t value;
	__asm__ __volatile__(
		"inl  %1, %0"
		:"=a"(value)
		:"Nd"(port)
		:
	);
	return value;
}
template <>     void send(uint16_t port,  uint8_t value) {
	__asm__ __volatile__(
		"outb  %0, %1"
		:
		:"a"(value), "Nd"(port)
		:
	);
}
template <>     void send(uint16_t port, uint16_t value) {
	__asm__ __volatile__(
		"outw  %0, %1"
		:
		:"a"(value),"Nd"(port)
		:
	);
}
template <>     void send(uint16_t port, uint32_t value) {
	__asm__ __volatile__(
		"outl  %0, %1"
		:
		:"a"(value),"Nd"(port)
		:
	);
}

/*void wait(void) {
	__asm__ __volatile__(
		"jmp 1f"
		"1:jmp 2f"
		"2:"
	);
}*/
void wait(void) {
	//Port 0x80 is used for "checkpoints" during POST.  The Linux kernel
	//	seems to think it is free for use :-/
	__asm__ __volatile__(
		"outb %%al, $0x80"
		:
		:"a"(0)
		:
	);
}


}}
