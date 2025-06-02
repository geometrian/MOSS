#include "serial.hpp"

#include "io/io.hpp"


namespace MOSS { namespace Serial {


#define PORT 0x3F8 //COM1

void init(void) {
	IO::send<uint8_t>(PORT+1, 0x00); //Disable all interrupts
	IO::send<uint8_t>(PORT+3, 0x80); //Enable DLAB (set baud rate divisor)
	IO::send<uint8_t>(PORT+0, 0x03); //Set divisor to 3 (low byte) 38400 baud
	IO::send<uint8_t>(PORT+1, 0x00); //                 (high byte)
	IO::send<uint8_t>(PORT+3, 0x03); //8 bits, no parity, one stop bit
	IO::send<uint8_t>(PORT+2, 0xC7); //Enable FIFO, clear them, with 14-byte threshold
	IO::send<uint8_t>(PORT+4, 0x0B); //IRQs enabled, RTS/DSR set
}

static bool _is_transmit_empty(void) {
	return IO::recv<uint8_t>(PORT + 5) & 0x20;
}
void write(uint8_t value) {
	while (!_is_transmit_empty());

	IO::send<uint8_t>(PORT,value);
}

static bool _serial_received(void) {
	return IO::recv<uint8_t>(PORT + 5) & 0x01;
}
uint8_t read(void) {
	while (!_serial_received());

	return IO::recv<uint8_t>(PORT);
}


}}