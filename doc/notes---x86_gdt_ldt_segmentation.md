# x86 Segmentation and the GDT and LDT

When you access a byte (whether code or data) on the x86 architecture, segmentation (and then
potentially also virtual memory / paging) applies.

Segmentation started as a way for 16-bit processors to use 20-bit addresses, and was also intended
for some kinds of virtual memory protection.  Today, it is considered obsolete even for 32-bit
processors, but unfortunately memory accesses on all x86 processors—16-, 32-, and 64-bit alike—still
invoke, if only nominally, memory segmentation.

Thus, we need to understand it.  It's really not so hard, although most treatments are confusing and
lack discussion of the point of it all.  A good overview is [on wikipedia][1] ([see also][2]), but
we'll rewrite and coalesce here.

[1]: https://en.wikipedia.org/wiki/X86_memory_segmentation
[2]: https://wiki.osdev.org/Segmentation



## 16-bit Real-Mode Segmentation

The [Intel 8086][20], a 16-bit microprocessor considered to be the first x86 processor, introduced
memory segmentation as a way to access 20-bit addresses (1 MiB of memory).

Basically, there are segment registers (the first four on the 8086, and all six on the [i386][21]
and beyond):
- CS, the code segment
- DS, the data segment
- SS, the stack data segment
- ES, an extra segment
- FS, another extra segment (i386+)
- GS, another extra segment (i386+)

On 16-bit architectures, these are 16 bits like everything else.  To compute the actual 20-bit
'linear address' of something, the processor takes the 16-bit address from the instruction and adds
the 16-bit segment shifted left by 4.  The multiples of 16 bytes thus indexed by the segment
register are called 'paragraphs':

	  1100 0110 1100 0011      <— Segment register, left-shifted 4
	+      0001 0010 1110 1001 <— Pointer, from code
	——————————————————————————
	  1100 0111 1111 0001 1001 <— Linear address (i.e. physical address)

(Note that, in 16-bit real mode, the linear address is always the physical address.  It was in
16-bit protected mode that virtual addressing was added, and the familiar virtual addressing scheme
of paging was added with the i386, for which is required (at least) 16-bit protected mode.  These
are discussed below.)

Since all memory accesses involve this scheme, all memory accesses involve a segment register.  In
assembly, usually, that register is implied by the operation.  E.g. `mov [si], ax` involves data, so
actually this is a write to the data segment, specified by DS.  The value will be written into
(DS<<4)+SI.  You can write the segment explicitly when you need to by using colons in the address.
E.g. the above is the same as `mov [ds:si], ax`.

Some programmers relied on the fact that this 20-bit address would wrap around to low addresses if
it overflowed.  To support this, when processor memories expanded further, the wrapping was usually
enabled by default by a disabled '[A20 gate][22]', which would mask off bits 20+ from the address.
Enabling this gate is an obnoxious relic of the past epitomizing the commonality in OS development
that poor decisions from the past endure indefinitely.

[20]: https://en.wikipedia.org/wiki/Intel_8086
[21]: https://en.wikipedia.org/wiki/I386
[22]: https://osdev.org/A20



## 16-bit Protected Mode

The [Intel 80286][30] ([see also docs][31] and in particular the surprisingly readable
[Programmer's Reference Manual][32], Ch. 6 (pg. 103 in PDF)) introduced 'protected mode', a layer on
top of segmentation.  It wasn't widely used because it would have introduced backward-compatibility
issues with the then-popular 8086.  The 80286 was also still 16-bit—and therefore the maximum
segment size was, too, limiting the point.  Still, it was added, and it forms the foundation we have
to deal with today.

### Segment Selector

Each segment register no longer indicates a paragraph.  Instead, it stores a
'[segment selector][33]' (see also Intel Arch Manual Vol. 3A §3.4.2 and 1 §3.4.2), whose basic
function is to index into a 'descriptor table'—either the '[global descriptor table][34]' ('GDT') or
a '[local descriptor table][35]' ('LDT'), discussed shortly.  Segment selectors look like this:

	        Segment Selector
	(meaning of each segment register)

	├─┴─┴─┴─┴─┴─┴─┴─┼─┴─┴─┴─┴─┼─┼─┴─┤
	│          Index          │T│PL │
	├───────────────┼─────────┼─┼───┤
	 15                      3 2 1 0

Bit 2 (the 'table indicator') selects the table, with 0 being the GDT and 1 being an LDT.  'Index'
is the index within the table.  'PL' is the 'privilege level', discussed below.

### Segment Descriptor in a Descriptor Table (GDT or LDT)

Meanwhile, each entry in a descriptor table is usually a 'segment descriptor', with the following
format (see Fig. 6-3 in the 286 manual, pg. 107 in PDF):

	                       Segment Descriptor
	     (most common entry in global or local descriptor table)

	                                │<─Access Byte─>│
	├─┴─┴─┴─┴─┴─┴─┴─┼─┴─┴─┴─┴─┴─┴─┴─┼─┼─┴─┼─┼─┴─┴─┼─┼─┴─┴─┴─┴─┴─┴─┴─┤
	│      Reserved (set to 0)      │P│DPL│S│Type │A│  Base 16–23   │
	├───────────────┼───────────────┼─┼───┼─┼─────┼─┼───────────────┤
	 63                           48 │     │ 43 41 │ 39           32
	                                 47    44      40

	├─┴─┴─┴─┴─┴─┴─┴─┼─┴─┴─┴─┴─┴─┴─┴─┼─┴─┴─┴─┴─┴─┴─┴─┼─┴─┴─┴─┴─┴─┴─┴─┤
	│           Base 0–15           │             Limit             │
	├───────────────┼───────────────┼───────────────┼───────────────┤
	 31                           16 15                            0

The 'Base' field is the start of a segment and the 'Limit' field is the highest offset within it (so
1 less than the size of the segment).

The 'Access Byte' (see Tab. 7-1 in the 286 manual, pg. 127 in PDF) says how the segment can be
accessed.  The 'DPL' ('descriptor privilege level') describes privilege levels required to access
the segment, with lower values indicating higher privilege (see below).  'P' ('present') must be 1
for any valid segment; it's not clear even from the contemporary manual what it's for.  'S'
indicates this is a segment descriptor (other entry types are possible but uncommon today, and so it
should be 1).  'A' ('accessed'), the processor will set to 1 when accessed—that can cause problems
if the descriptor table is itself in read-only memory, which it probably should be, so better to
leave it 1.

The 'Type' field determines code vs. data and read / write privileges.  Specifically, the upper bit
(bit 43 overall) has the meaning 0=data, 1=code.  The last bit (bit 41) determines whether it can be
accessed, with 0 meaning the code *can't* be read or data written, and 1 meaning the code *can* be
read or the data written.  The middle bit (bit 42) determines whether the
[segment expands downward][36] (it should be 0 for sanity).

### How it Works Together

Now that we have all the machinery, we can understand how it's supposed to work.

First, the OS sets up a GDT, whose constituent segment descriptors describe memory segments
accessible system-wide.  For each process, the OS also creates an LDT, describing memory segments
specific to that process.  (You have to create a GDT, but you don't necessarily have to create
LDT(s); this is just an idea.)

When a process makes a memory access, one of the segment registers is involved.  This in turn holds
a segment selector.  The processor goes to the indicated table (either the global GDT or the
process's LDT), and looks up the indicated segment descriptor.  If everything is in order, the
address from the instruction gets added to the base from the descriptor and now we have a linear
address, through which the processor can begin to access memory (this is likely still a physical
address, but it could be virtual if the processor supports paging).  At each stage there is
checking, thus accomplishing different features and granularities of memory protection and
virtualization.

The GDT is stuff that is usable system-wide.  For example, it might be a common scratch space, or
code that can be called from any process (e.g. a standard library of utility functions).  However,
the LDT is specific to each process and, if the OS sets it up right, the LDT-described segments from
separate processes can be completely separate in memory.  Program accesses to segments described by
their LDTs will never overlap, enforcing protection.

All accesses are also relative to the base offsets in the GDT or LDT, meaning that programs have a
virtual address space.  One process's address DS:0000 in its LDT will be in a different location
from another process's DS:0000 in its LDT, simply because the base offsets in the separate
processes' separate LDTs would be different.  However, the processes themselves wouldn't have to
care, nor have to be careful of each other's address space.

The privilege level in the CS (i.e. code-segment) register is the 'current privilege level' ('CPL'),
and so describes the current privilege level of the code.  There is a privilege check when trying to
load a new segment selector into any other segment register (see Intel Vol. 3A §5.6).  The new
segment selector's privilege level is the 'requested privilege level' ('RPL').  The processor looks
up the segment descriptor and its privilege level (DPL) from the LDT or GDT, and ensures that:

	Requested Privilege Level        Descriptor Privilege Level (from
	(from the segment selector       the segment descriptor referenced
	you're trying to load)           by that new segment selector)
	                    \                /
	                    RPL ≥ max( CPL, DPL )
	                               /
	            Current Privilege Level
	            (from CS's segment selector)

That is, you can only load (and therefore use) a new segment selector into a (non-code-)segment
register if the new segment selector does not represent a higher privilege than the current
privilege level.  Thus, processes cannot access privileged data.  Of course, the segment descriptor
also has to be a valid index in the table, be present, and actually be a segment descriptor, too.

[30]: https://en.wikipedia.org/wiki/Intel_80286
[31]: http://bitsavers.org/components/intel/80286/
[32]: http://bitsavers.org/components/intel/80286/210498-003_iAPX_286_Programmers_Reference_1985.pdf
[33]: https://wiki.osdev.org/Segment_Selector
[34]: https://wiki.osdev.org/Global_Descriptor_Table
[35]: https://wiki.osdev.org/Local_Descriptor_Table
[36]: https://wiki.osdev.org/Segment_Limits#Expand_Down



## 32-bit Protected Mode and (64-bit) Long Mode

32-bit protected mode is similar to 16-bit protected mode.  The main difference is that the segment
descriptors are [somewhat][40] more powerful:

	                       Segment Descriptor
	     (most common entry in global or local descriptor table)

	                G D/B L
	                └┐ │ ┌┘         │<─Access Byte─>│
	├─┴─┴─┴─┴─┴─┴─┴─┼─┴─┴─┴─┼─┴─┴─┴─┼─┼─┴─┼─┼─┴─┴─┼─┼─┴─┴─┴─┴─┴─┴─┴─┤
	│  Base 24–31   │ Flags │ Limit │P│DPL│S│Type │A│  Base 16–23   │
	├───────────────┼───────┼───────┼─┼───┼─┼─────┼─┼───────────────┤
	 63           56 55   52 51   48 │     │ 43 41 │ 39           32
	                                 47    44      40

	├─┴─┴─┴─┴─┴─┴─┴─┼─┴─┴─┴─┴─┴─┴─┴─┼─┴─┴─┴─┴─┴─┴─┴─┼─┴─┴─┴─┴─┴─┴─┴─┤
	│           Base 0–15           │          Limit 0–15           │
	├───────────────┼───────────────┼───────────────┼───────────────┤
	 31                           16 15                            0

The 'Limit' field has been extended to 20 bits (split up), and the 'Base' field extended to 32 bits
(split up).  The 'Access byte' is as before.

The 'Flags' field is where it gets interesting.  'G' ('granularity') is, intuitively, used to scale
'Limit' (but note, not 'Base').  0 is the old meaning, byte.  However, 1 means 4 kiB.  An address's
lower 12 bits are ignored, and the remainder is compared against the limit field to determine if it
is valid.  Thus, setting 'G' to 1 and 'Limit' to 0xFFFFF results in the entire 4 GiB addressable
space being used.  (Also note: because of the definition, setting 'Limit' to zero, the addresses
0—4095 are *still valid*.)

The 'D/B' flag ('default' / 'big') specifies a large / big (i.e. 32-bit) default operand size for
code and data size for memory operations.  In short, 0 means the segment is 16-bit protected mode
and 1 means the segment is 32-bit protected mode.  'L' ('long mode') can be set *(instead)* of D/B
to select 64-bit long mode (see below).  And the last bit of 'Flags' is reserved.

In long mode, segmentation has been as stripped away as possible, and the 'Base' and 'Limit' are
almost unused ("almost" because the limit needs to be 0xFFFFF, or the jump to 64-bit will fail).
Memory access must be through paging.  Nevertheless, the GDT still needs to exist.  Also note that
in long mode, the segment descriptor is [essentially the same][41] segment descriptor is used.  In
particular, the segment descriptor is still 8 bytes large.  (Other entries in the LDT may be larger,
but we won't worry about that, since the LDT oughtn't to be used.)

[40]: https://wiki.osdev.org/Global_Descriptor_Table#Segment_Descriptor
[41]: https://wiki.osdev.org/GDT_Tutorial#Flat_.2F_Long_Mode_Setup
