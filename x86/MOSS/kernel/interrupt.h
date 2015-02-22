#pragma once

namespace INTERRUPT {

void disable(void) {
	asm("cli");
}
void enable(void) {
	asm("sti");
}

}