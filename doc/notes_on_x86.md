
https://os.phil-opp.com/red-zone/


## Operating Modes

╔═════════════════════════════╦═════════════════════╗
║ Operating                   ║ Code                ║
╟────────┬────────────────────╢                     ║
║ Mode   │ Sub-mode           ║                     ║
╠════════╪════════════════════╬═════════════════════╣
║ Long   │ 64-bit mode        ║ 64-bit              ║
║ Mode   ├────────────────────╫─────────────────────╢
║        │ Compatibility mode ║ 32-bit              ║
║        │                    ║ 16-bit (protected)  ║
╟────────┼────────────────────╫─────────────────────╢
║ Legacy │ Protected mode     ║ 32-bit              ║
║ Mode   │                    ║ 16-bit (protected)  ║
║        ├────────────────────╫─────────────────────╢
║        │ Virtual 8086 mode  ║ 16-bit real mode    ║
║        │                    ║ (subset)            ║
║        ├────────────────────╫─────────────────────╢
║        │ Unreal mode        ║ 16-/20-/32-bit      ║
║        │                    ║ real mode           ║
║        ├────────────────────╫─────────────────────╢
║        │ Real mode          ║ 16-/20-/21-bit      ║
║        │                    ║ real mode           ║
╚════════╧════════════════════╩═════════════════════╝

([source](https://en.wikipedia.org/wiki/X86-64#Operating_modes))



## The Stack

(See [The Stack](notes---x86_stack.md).)



## Memory

x86 has two virtual memory schemes—

	segmentation, must, managed using the segment table, GDT.
	paging, optional, managed using the page table, PDT.

Most operating systems want to to use paging and don't want the segmentation, but its must and can't
just be disabled.

So the trick is to disable its effect as it wasn't there. This can usually be done by creating 4
large overlapped segments descriptors (beside the null segment):

	segment index 0 : null segment descriptor
	segment index 1 : code segment descriptor for the privileged (kernel) mode
	segment index 2 : data segment descriptor for the privileged (kernel) mode
	segment index 3 : code segment descriptor for the non-privileged (user) mode
	segment index 4 : data segment descriptor for the non-privileged (user) mode

https://wiki.osdev.org/Segmentation

[](https://stackoverflow.com/a/23979175/)



## Page Tables

TODO: diagram for various 32-bit modes
([source](https://en.wikipedia.org/wiki/Physical_Address_Extension#Page_table_structures))

Virtual ("linear") address

	 63   56 55   48 47   40 39   32 31   24 23   16 15    8 7     0
	├┬┬┬┬┬┬┬┼┬┬┬┬┬┬┬┼┬┬┬┬┬┬┬┼┬┬┬┬┬┬┬┼┬┬┬┬┬┬┬┼┬┬┬┬┬┬┬┼┬┬┬┬┬┬┬┼┬┬┬┬┬┬┬┤
	├┴┴┴┴┴┴┴┴┴┴┴┴┴┴┴┼┴┴┴┴┴┴┴┴┼┴┴┴┴┴┴┴┴┼┴┴┴┴┴┴┴┴┼┴┴┴┴┴┴┴┴┼┴┴┴┴┴┴┴┴┴┴┴┤
	 ─<16>────────── ─<9>──── ─<9>──── ─<9>──── ─<9>──── ─<12>──────
	   │               │        │        │        │        │
	  unused           │ ╔═══╗  │ ╔═══╗  │ ╔═══╗  │ ╔═══╗  │ ╔═══╗
	  (sign ext)       │ ║   ║  │ ║   ║  │ ║   ║  │ ║   ║  │ ║   ║
	                   │ ║...║  │ ║...║  │ ║...║  │ ║...║  │ ║...║
	                   │ ║   ║  │ ║   ║  │ ║   ║  │ ║   ║  │ ║   ║
	                   │ ╟───╢  │ ╟───╢  │ ╟───╢  │ ╟───╢  │ ╟───╢
	                   │ ║  ─╫─┐│ ║  ─╫─┐│ ║  ─╫─┐│ ║  ─╫─┐│ ║  ─╫───<8>───> value
	                   └>╟───╢ │└>╟───╢ │└>╟───╢ │└>╟───╢ │└>╟───╢
	   ╔═════╗           ║...║ │  ║...║ │  ║...║ │  ║...║ │  ║...║
	   ║ CR3 ╫───<40>───>╚═══╝ └─>╚═══╝ └─>╚═══╝ └─>╚═══╝ └─>╚═══╝
	   ╚═════╝             ┊        ┊        ┊        ┊        ┊
	    PML4 table ╌╌╌╌╌╌╌╌╯        ┊        ┊        ┊        ┊
	    page-directory-ptr table ╌╌╌╯        ┊        ┊        ┊
	    page directory ╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╯        ┊        ┊
	    page table ╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╯        ┊
	    4 kiB page ╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╯

([source](https://en.wikipedia.org/wiki/X86-64#Page_table_structure))
