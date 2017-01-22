#include "includes.h"

#include "kernel/graphics/vga/terminal.h"
#include "kernel/kernel.h"
#include "kernel/serial.h"

#include "mossc/cstdio"
#include "mossc/cstring"


extern "C" void die(void) {
	//Clear hardware interrupts (to prevent the next instruction being interrupted) and then "hlt".
	__asm__ __volatile__("cli");
	__asm__ __volatile__("hlt");
	//If that fails, busy loop.
	LOOP: goto LOOP;
}

#if UINT32_MAX == UINTPTR_MAX
	#define STACK_CHK_GUARD 0x73D04232u
#else
	#define STACK_CHK_GUARD 0x83EB3E889E3F1941ull
#endif
uintptr_t __stack_chk_guard = STACK_CHK_GUARD;
extern "C" __attribute__((noreturn)) void __stack_chk_fail(void) {
	assert_term(false,"Stack smashing detected!");
	while (1); //For the noreturn
}

namespace MOSS {


#ifdef MOSS_DEBUG
static void _write_serial(char const* buffer) {
	char c;
	int i = 0;
	LOOP:
		c = buffer[i];
		if (c!='\0') {
			Serial::write(c);
			++i;
			goto LOOP;
		}
}
#endif
void _message(char const* filename,int line, char const* fmt_cstr,va_list args) {
	int len_this = MOSSC::strlen(__FILE__);
	filename += len_this - 24;

	Graphics::VGA::Terminal::Color temp;
	if (kernel->terminal!=nullptr) {
		temp = kernel->terminal->color_text;
		kernel->terminal->set_color_text(Graphics::VGA::Terminal::Color::RED);
	}

	char buffer[1024]; //TODO: vsnprintf
	MOSSC::sprintf(buffer, "(%s:%d): ",filename,line);
	#ifdef MOSS_DEBUG
	_write_serial(buffer);
	#endif
	if (kernel->terminal!=nullptr) {
		kernel->write("%s", buffer);
	}

	MOSSC::vsprintf(buffer, fmt_cstr,args);
	#ifdef MOSS_DEBUG
	_write_serial(buffer);
	#endif
	if (kernel->terminal!=nullptr) {
		kernel->write("%s", buffer);
	}

	#ifdef MOSS_DEBUG
	_write_serial("\n");
	#endif
	if (kernel->terminal!=nullptr) {
		kernel->write("\n");
	}

	if (kernel->terminal!=nullptr) {
		kernel->terminal->set_color_text(temp);
	}
}

//TODO:
//	Would be nice to get symbols here.  The ELF format the MOSS kernel is stored in uses the standard Dwarf debugging symbols.
//	While this is (probably?) a fairly straightforward thing to parse, the only references are things like glibc--too bloated
//	to be terribly useful.  See:
//		http://www.ibm.com/developerworks/library/os-debugging/
/*static void this_is_function_e(uint32_t max_frames) {
	//http://www.hexblog.com/?p=104

	Start by retrieving the stack pointer register value (for the current thread) and its associated segment
	From the stack pointer to the upper limit of the stack segment:
		Take a Dword
		Check if it belongs to an executable segment, if so then it is probably a code pointer (exception handler, callback pointer, or return address)
		Try to determine if the value at the stack pointer is a return address (we try to find the beginning of the previous instruction and we decode it to see if it is a CALL instruction)
		Once we have a CALL instruction we will try to build a nice expression to represent the call stack:
			If it belongs to a function then use the following name: function name+offset
			Otherwise try to check nearest debug name (exported names) and use the following name: nearest_debug_name+offset 
		Save the address (for later use) 
	Finally render the results (in a chooser, message window, etc…) 


	//Stack contains:
	//	First function argument ("max_frames")
	//	Return address in calling function
	//	"ebp" of calling function (pointed to by current ebp)
	uint32_t* ebp = &max_frames - 2;
	kernel->write("Stack trace:\n");
	for (uint32_t frame=0; frame<max_frames; ++frame) {
		uint32_t eip = ebp[1];
		if (eip == 0) { //No caller on stack
			break;
		}
		//Unwind to previous stack frame
		ebp = reinterpret_cast<uint32_t*>(ebp[0]);
		uint32_t* arguments = &ebp[2];
		kernel->write("  Entry: 0x%X\n",eip);
	}
	kernel->write("Complete\n");
	while (1);
}
static void this_is_function_d(void) {
	this_is_function_e(16u);
}
static void this_is_function_c(void) {
	this_is_function_d();
}
static void this_is_function_b(void) {
	this_is_function_c();
}
static void this_is_function_a(void) {
	this_is_function_b();
}
void stack_trace(void) {
	this_is_function_a();
}*/
class StackFrame final { public:
	StackFrame* next;
	void* ret;
};
int _get_call_stack(void** retaddrs, int max_size) {
	//x86/gcc-specific: this tells gcc that the fp
	//	variable should be an alias to the %ebp register,
	//	which keeps the frame pointer.
	register StackFrame* fp __asm__("ebp");

	//Walks through the linked list
	StackFrame* frame = fp;
	int i = 0;
	while (frame!=nullptr) {
		if (i<max_size) {
			retaddrs[i++] = frame->ret;
		}
		frame = frame->next;
	}
	return i;
}
void stack_trace(void) {
	//http://yosefk.com/blog/getting-the-call-stack-without-a-frame-pointer.html

	void* addrs[64];
	int size = _get_call_stack(addrs,64);
	for (int i=0;i<size;++i) {
		kernel->write("  0x%p\n",addrs[i]);
	}
}


}
