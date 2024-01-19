#include "types.hpp"

#include "../../kernel.hpp"


namespace MOSS { namespace Interrupts {


void InterruptState::write(void) const {
	kernel->write("eip: 0x%p, eflags: %u, useresp: %u\n",eip,eflags,useresp);
	kernel->write("cs: 0x%p    eax: 0x%p    edi: 0x%p\n",cs,eax,edi);
	kernel->write("ds: 0x%p    ebx: 0x%p    esi: 0x%p\n",ds,ebx,esi);
	kernel->write("es: 0x%p    ecx: 0x%p    ebp: 0x%p\n",es,ecx,ebp);
	kernel->write("fs: 0x%p    edx: 0x%p    esp: 0x%p\n",fs,edx,esp);
	kernel->write("gs: 0x%p\n",gs);
	kernel->write("ss: 0x%p\n",ss);
	kernel->write("debug: 0x%p\n",debug_marker);
	kernel->write("interrupt: %d\n",int_index);
	kernel->write("error code: %b\n",error_code.packed);
}


}}
