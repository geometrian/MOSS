# x86 Exceptions and Interrupts

The best high-level reference on this is _Understanding the Linux Kernel 3rd Ed._, Ch. 4 (pg.
131–188).  Vital sections include [§4.2][1].

Note: unfortunately, the terminology around this topic is confusing—and often ambiguous among
authors because precise language is not a dispositional virtue of system programmers.  In fairness,
it is difficult to find clear information, and the terms overlap with more prevalent but different
concepts in non-systems programming.  Throughout, I'll be consistent, and select the terms with the
best credibility, defining as we go.

An 'interrupt-signal' is a signal to the CPU, ultimately from a device (e.g. the keyboard, hard
drive, or the CPU itself), telling the CPU to stop what it's doing and do something else.  For
example, if a key is pressed, the keyboard interrupts the CPU to tell it to read the key.

Interrupt-signals come in two types, which Intel calls 'exceptions' or 'interrupts'.  Exceptions,
AKA 'software interrupts' or 'synchronous interrupts', are generated from code execution.
Interrupts, unqualified, AKA 'hardware interrupts' or 'asynchronous interrupts', ultimately
originate externally to the CPU.  For the least ambiguity, I will contrast these as 'exception' vs.
'hardware interrupt' and call them collectively 'interrupt-signals'.

Hierarchy of the terminology; the primary term I recommend and will use comes first, with others
coming after '/'s:

- **Interrupt-Signal** / Interrupt
	- **Exception** / Software Interrupt / Synchronous Interrupt
		- **Processor-Detected Exception** / Processor-Detected Program Error
			- **Fault**
			- **Trap**
			- **Abort**
		- **Programmed Interrupt** / Software-Generated Interrupt
		- **Machine Check**
	- **Hardware Interrupt** / Interrupt / Asynchronous Interrupt
		- **Maskable Interrupt**
			- **I/O Interrupt**
			- **Timer Interrupt**
			- **Interprocessor Interrupt**
		- **Non-maskable Interrupt** (NMI)

Maskable interrupts can be enabled with the [`sti` instruction][2] and disabled with the
[`cli` instruction][3].  When BIOS boots, they [are enabled][4], as is the case with UEFI
[for IA-32][5] and [for x86-64][6].  However, most bootloaders (for example, [in BOOTBOOT][7],
[in GRUB Legacy][8] (see IF=0), [in GRUB 2 32-bit][9], and implied not to be but probably also
[in GRUB 2 64-bit][10]) hand the state over with maskable interrupts disabled.  They should be /
remain disabled until the OS is ready to handle them (e.g. until the GDT and IDT are configured).

([See also.][11])

[1]: https://www.oreilly.com/library/view/understanding-the-linux/0596005652/ch04s02.html
[2]: https://www.felixcloutier.com/x86/sti
[3]: https://www.felixcloutier.com/x86/cli
[4]: https://wiki.osdev.org/Babystep1#Code
[5]: https://uefi.org/specs/UEFI/2.10/02_Overview.html#ia-32-platforms
[6]: https://uefi.org/specs/UEFI/2.10/02_Overview.html#x64-platforms
[7]: https://gitlab.com/bztsrc/bootboot/-/tree/master#machine-state
[8]: https://www.gnu.org/software/grub/manual/multiboot/multiboot.html#Machine-state-1
[9]: https://www.gnu.org/software/grub/manual/multiboot2/multiboot.html#I386-machine-state
[10]: https://www.gnu.org/software/grub/manual/multiboot2/multiboot.html#EFI-amd64-machine-state-with-boot-services-enabled
[11]: http://www.brokenthorn.com/Resources/OSDev15.html


## Interrupt-Signal Handling

By the time the [interrupt-signal][20] is considered by the CPU, it is identified by a type—a number in
the range 0–255 called the 'vector number' (or more commonly but less clearly, 'interrupt-vector').
The interrupt-vector for any exception or 'non-maskable interrupt' ('NMI') is fixed, while the
vectors for maskable interrupts can be changed.

### Interrupt Table

The interrupt-vector value indexes into a CPU table.  The basic information provided by each table
entry is a function pointer to an '[interrupt service routine][21]' ('ISR'): a function that should
be called when the corresponding interrupt-signal comes in.  Control transfers to the ISR, which
handles the interrupt-signal appropriately, and then returns to whatever was executing before.
Usually, that's a user-space process, while the ISR runs in kernel-space, so a change of privilege
level is implied in both the call and return.

It is common to deal with the complexity by making most ISRs delegate to a common routine.  ISRs may
or may not be passed an error code, depending on the type of the interrupt-signal (see subsequent
sections), and to deal with this complexity, it is common to push a dummy code if one is not
required, and just call one function.  Note that due to automatic stack alignment, you only need
push 1 byte, though this is confusing ([see also][22]).

### IVT and IDT

In x86 16-bit real mode, the table is called the '[interrupt-vector table][23]' ('IVT').  Each 4-byte
entry contains (as a segment and offset) only the function pointer to the ISR.  (We won't discuss
16-bit further, though.)

In x86 32-bit protected and 64-bit long modes, the table is called the
'[interrupt-descriptor table][24]' ('IDT').  Each entry is called a 'gate', and the stored data
is called the 'gate-descriptor'.  In addition to the function pointer to the ISR, gate-descriptors
also specify a type, as well as (for exceptions) the highest ring that is allowed to call it.

### IDT Gate Types

The gate types for 32-bit are confusingly named 16-bit trap-gate, 16-bit interrupt-gate, 32-bit
trap-gate, 32-bit interrupt-gate, or task-gate.  In 64-bit, the only types are 64-bit trap-gate and
64-bit interrupt-gate.  The task-gate type was intended for hardware task switching, but was not
very good at it.  Thus, the main gate types are trap- vs. interrupt-gate.

The name is confusing because trap-gates don't have to correspond to trap exceptions and
interrupt-gates don't have to correspond to hardware interrupts.  (Albeit, trap-gates *should
generally* be used for *exceptions* and interrupt-gates for hardware interrupts.)

The difference?  A trap-gate only calls the ISR when the interrupt-signal comes in.  However, an
interrupt-gate also masks (disables) hardware interrupts when it enters the ISR (and also unmasks
(re-enables) them when the ISR returns).  This is critically important because it means that an
interrupt-gate's ISR will not be interrupted by more interrupt-signals (unless the ISR chooses to
re-enable them early, [see also][25]).  This allows interrupt-gate ISRs to function atomically,
which is necessary for correctly implementing certain operations.  (See also Intel Vol. 3A
§6.12.1.3.)

Exceptions always go to gates 0–31, and their interrupt-vectors (and presumably, required gate
configuration) are architecturally defined.  Whereas, all hardware interrupts (should) be configured
to go to gates 32–255 (discussed in that section).

[20]: https://wiki.osdev.org/Interrupts
[21]: https://wiki.osdev.org/Interrupt_Service_Routines
[22]: https://wiki.osdev.org/Interrupt_Vector_Table
[23]: https://forum.osdev.org/viewtopic.php?f=1&t=23998
[24]: https://wiki.osdev.org/Interrupt_Descriptor_Table
[25]: https://forum.osdev.org/viewtopic.php?f=1&t=20572



## Exceptions / Software Interrupts / Synchronous Interrupts

Exceptions represent an anomalous condition.  Some types push an error code onto the stack, while
others don't.

### Faults, Traps, Aborts

Processor-detected exceptions (AKA 'processor-detected program "error"', with "error" quoted, since
it often does not indicate an actual error) are categorized by the way the return is handled.

For a 'fault', the saved instruction pointer is the faulting instruction.  The idea is that the
instruction can be retried after the kernel fixes something up behind the scenes.  For example, in a
page fault, the faulting instruction accessed non-mapped memory.  It can be retried after the kernel
has paged the memory in.  For a 'trap', the saved instruction pointer is right *after* the
originating instruction.  It is mainly used for debuggers, as discussed below.  For an 'abort', the
saved instruction pointer is unreliable, and it is expected that the process should terminate.

### Programmed Exceptions

A programmed exception is usually just executing the `int` instruction, and it can be a fault, trap,
or abort depending on what the corresponding gate specifies.  It can be used to simulate an
interrupt-signal of another type, including of a hardware interrupt, although no error code is
passed even if it's an exception and expects one.  The `bound` and `into` instructions also are
programmed exceptions.

One main use for `int` is implementing syscalls.  Another is for implementing debuggers; `int 3` is
specifically designed for this.  Literally, the instruction encoding is `0xcd 0x03`—but it also has
a variant dedicated 1-byte encoding `0xcc`.  Because that is only one byte, it can replace any
instruction.  When executed, the trap handler can replace the breakpoint by the actual instruction
and continue, thus implementing a breakpoint.

### Machine Check

Finally, the machine check exception is raised when hardware detects a problem with itself.  This
exception is [not enabled by default][30].

[30]: https://wiki.osdev.org/Exceptions#Machine_Check



## Hardware Interrupts / Interrupts / Asynchronous Interrupts

Hardware interrupts generally represent that a hardware device wants the kernel's attention.  Unlike
exceptions, hardware interrupts never pass error codes.

When a hardware device wants to interrupt the CPU, it sends an 'interrupt request' ('IRQ').  The IRQ
arrives at the 'chipset', which connects the CPU to the rest of the system.  (In x86 terms: the
'chipset' is the northbridge + southbridge combo that the CPU connects to through the 'front-side
bus', even if, in practice, major portions of this are [now included on the CPU][40].)  The incoming
IRQ is then translated by the chipset into a 'hardware interrupt', which is then passed to the CPU.

For example, for a classic PS/2 keyboard and BIOS, pressing a key causes an IRQ ([IRQ #1][41],
[on the ISA bus][42], see also [another list of IRQs][43]) to the chipset.  The chipset then raises
a hardware interrupt (interrupt-vector 9, by default) to the CPU.

### Programmable Interrupt Controller (PIC)

The translation of hardware IRQs to interrupt-vectors is handled by the chipset—and particularly on
x86, by (at least by default) a (probably emulated) (probably pair of, but potentially more)
[Intel 8259][44] 'programmable interrupt controller' ('PIC') chip(s) ([datasheet][45]).  These
simply add a stored offset (which must be a multiple of 8) to the index of the input IRQ to form the
interrupt-vector number.  If there are multiple IRQs, the lower-numbered one takes priority.

The old, one-chip configuration is the 'IBM PC 8259 PIC Architecture'.  Modern systems use (more
likely, emulate) the two-chip 'IBM PC/AT 8259 PIC Architecture'.  In this latter configuration, the
chips [are cascaded][46] so that IRQ 2 in the primary PIC is fed by the secondary PIC.  IRQs 0–7
(exclusive of IRQ 2, which is used for chaining; anything that thinks it uses IRQ 2 is actually
using IRQ 9, for some reason) are handled by the primary PIC with (by default) offset 0x08 (8),
while IRQs 8–15 are handled by the secondary PIC with offset 0x70 (112).  Note that because lower
numbers take priority and IRQ 2 is used for chaining, the priorities run, decreasing: 0, 1, 8, 9,
10, 11, 12, 13, 14, 15, 3, 4, 5, 6, 7.

	Interrupt <────┬───────────────────────────┐
	Vector         │                           │
	         0x08–0x0f\0x0a                0x70–0x77
	           (8–15\10)                   (112–119)
	               │                           │
	            ╔══╪═══════════╗            ╔══╪═══════════╗
	            ║  │     Index ║            ║  │     Index ║
	            ║ Add <─┬─ 0 ──╫─ IRQ 0     ║ Add <─┬─ 0 ──╫─ IRQ  8
	            ║ 0x08  ├─ 1 ──╫─ IRQ 1     ║ 0x70  ├─ 1 ──╫─ IRQ  9
	            ║ (8)   ├─ 2 ──╫─<────────┐ ║ (112) ├─ 2 ──╫─ IRQ 10
	            ║       ├─ 3 ──╫─ IRQ 3   │ ║       ├─ 3 ──╫─ IRQ 11
	            ║       ├─ 4 ──╫─ IRQ 4   │ ║       ├─ 4 ──╫─ IRQ 12
	            ║       ├─ 5 ──╫─ IRQ 5   │ ║       ├─ 5 ──╫─ IRQ 13
	            ║       ├─ 6 ──╫─ IRQ 6   │ ║       ├─ 6 ──╫─ IRQ 14
	Interrupt <─╫─ INT  └─ 7 ──╫─ IRQ 7   └─╫─ INT  └─ 7 ──╫─ IRQ 15
	Line        ╚══════════════╝            ╚══════════════╝
	            Primary 8259                Secondary 8259

Thus, IRQs 0–7 (excl. 2) are inputs 0–7 (excl. 2) of the primary, which adds 0x08 to form
interrupt-vectors 0x08–0x0f (8–15) (excl. 0x0a (10)).  Meanwhile, IRQs 8–15 are inputs 0–7 of the
secondary, which adds 0x70 to form interrupt-vectors 0x70–0x77 (112–119).  Interrupt-vectors 0–31
are reserved by Intel, so this mapping "was an IBM design mistake".  Fortunately, you can remap the
PIC(s) to translate to different interrupt-vector numbers—the usual choice is to set the primary's
offset to 0x20 (32) and the secondary's offset to 0x28 (40).  Thus, IRQs 0–15 map to
interrupt-vectors 32–47.

[40]: https://en.wikipedia.org/wiki/Chipset#Move_toward_processor_integration_in_PCs
[41]: https://en.wikipedia.org/wiki/Interrupt_request#x86_IRQs
[42]: https://wiki.osdev.org/Interrupts#General_IBM-PC_Compatible_Interrupt_Information
[43]: https://en.wikipedia.org/wiki/Interrupt_request#x86_IRQs
[44]: https://wiki.osdev.org/PIC
[45]: http://pdos.csail.mit.edu/6.828/2005/readings/hardware/8259A.pdf
[46]: https://masherz.wordpress.com/2010/08/15/pic-8259/
