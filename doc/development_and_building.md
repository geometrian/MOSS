# Development and Building



## Goals

MOSS is a simple DIY OS, created for fun.  The main aim of the code is to provide clarity, as well
as to some degree performance and robustness.  However, it does *not*, at least currently, aspire to
great security, compatibility, or featurefulness.



## Architecture

The basic architecture aims to be an extreme microkernel, meaning that the kernel itself is a very
small and simple layer over the hardware, and most of the actual functionality is provided by
userland system libraries.  This has tradeoffs, but the consensus is that microkernels are better
overall.  One major reason is security: a smaller kernel is a smaller attack surface.  For our
purposes, more important is robustness.  It also starts faster and is in some ways simpler.

It also aims to be an exokernel, meaning that the kernel offers very direct access to the hardware,
to allow programmers to largely build their own abstractions.



## Language and Standards

The MOSS kernel is written in C23.  C++23 is a strictly superior language, but to get most of the
benefit we'd need a C++ standard library in addition to the C standard library.  Although this is
implemented, it adds significantly to the in-kernel complexity.  Another practical reason is that
exceptions are not really viable in the kernel, and static constructors / destructors are a pain to
set up.  So, the main utilities of MOSS are written in C++, but not the kernel per-se.  All code is
standard-compliant.

The code itself adheres to a standard of clarity.  The code is written clearly instead of tersely,
and there are extended comments everywhere, using extensive ASCII art (actually UTF-8, so it looks
even better).  Nesting is also avoided—checks and early returns for failed conditions are (usually
though not always) better than nesting passed conditions—even if they aren't very deep.

Inline assembly is necessary for technical reasons, but it's generally avoided when possible.  It's
not because we're scared of it.  It's because the compiler doesn't understand it very well, and it
optimizes worse (e.g. it prevents constant-expression simplification).  We do use a lot of `goto`,
and [this is perfectly fine][1].

TAB is used to indent and spaces are used to align, because this is
[objectively the semantically correct choice][2].  We align stuff aggressively and also space
liberally; these help, visually.  All lines are wrapped manually at 100 characters: 80 characters is
obviously too small for modern screens, but shorter lines are still better in general, especially
when putting multiple files side-by-side.  If an occasional line goes over 100 characters, and
refactoring it would impede clarity, we don't do that, though.

Braces go on the next line, unindented.  Most programmers (including myself) started with braces on
the same line, but through experience I have come to see that putting it on the next line is
superior.  It breaks up control flow logic visually.  The biggest criticism is that it takes up more
lines of code, but that is actually a feature, because it encourages factoring into smaller pieces
and using simpler logic.  You can fit a lot of lines on a modern screen anyway.

    [1]: https://geometrian.com/projects/blog/the_goto_statement_is_good_actually.html
    [2]: https://geometrian.com/projects/blog/tabs_versus_spaces.html



## Environment Setup (INCOMPLETE)

1. Get UEFI headers.  Clone [tianocore EDK II](https://github.com/tianocore/edk2/tree/master).
   Change `UEFI_ROOT` in "src/Makefile" to point to that repo's "edk2/".



## BOOTBOOT

https://osdev.org/BOOTBOOT

https://gitlab.com/bztsrc/bootboot/-/tree/master

https://gitlab.com/bztsrc/bootboot/raw/master/dist/bootboot.efi
https://gitlab.com/bztsrc/bootboot/raw/master/dist/bootboot.rom



## Limine



## Setup (INCOMPLETE / OBSOLETE)

In particular, we don't seem to need a cross-compiler anymore.

### Option 1: Linux-only setup:

1. Update/upgrade everything
(`sudo apt-get update`/`sudo apt-get upgrade`/`sudo apt-get dist-upgrade`)
2. Install misc. necessary packages: `sudo apt-get install build-essential nasm git kpartx grub`
2. In a folder somewhere, check out `MOSS`.  E.g.:
`cd ~; git clone https://github.com/geometrian/MOSS`.
3. [Set up a cross-compiler](doc/cross_compiler.md).
4. Install tools and emulators: `sudo apt-get install qemu-kvm bochs-x`.
5. Build MOSS.  E.g. `cd source/x86/; ./scripts/all.sh`.
6. Run it!  E.g. `python _run_bochs.py`/`python _run_qemu.py`.
7. [If you want to also set up VirtualBox](VMs/README.md).

### Option 2: Windows + VirtualBox Linux VM setup:

1. Set up and install Linux into a VirtualBox x86-64 VM.  Note that an 8GB HDD is too small to
handle both an e.g. Ubuntu 16.10 MATE installation and a GCC compile.  I recommend at least 16GB,
ensuring that there's no swap file eating half of it.
2. Update/upgrade everything
(`sudo apt-get update`/`sudo apt-get upgrade`/`sudo apt-get dist-upgrade`)
3. Install misc. necessary packages: `sudo apt-get install build-essential nasm git kpartx grub`
4. Insert guest-additions CD into VM and follow the autorun prompt.
5. Add yourself to share folders: `sudo adduser <username> vboxsf`
6. Create a shared folder.
7. Reboot.
8. In the shared folder, check out `MOSS`.  E.g.:
`cd /media/sf_dev/; git clone https://github.com/geometrian/MOSS`.
9. [Set up a cross-compiler](doc/cross_compiler.md).
10. Install tools and emulators: `sudo apt-get install qemu-kvm bochs-x`.
11. Build MOSS.  E.g. `cd source/x86/; ./scripts/all.sh`.
12. Run it!  E.g. `python _run_bochs.py`/`python _run_qemu.py`.
13. [If you want to also set up VirtualBox](doc/virtual_machines.md).

### Option 2: (Additional Tweaks)

I do MOSS development using the Windows + Linux VM setup above.  The following makes that easier:

- [Visual Studio](https://www.visualstudio.com/) is used for the project files.  Note that (the
real version of) Visual Studio currently only runs on Windows.
- [Python](https://www.python.org/) is assumed to ship with Linux, as above, but on Windows you
have to install it if you want to run any of the Python scripts.  The scripts should be compatible
with either major version.
- The virtualization softwares installed above can also be installed on Windows.  If you do so, the
Python scripts can be run, and the virtualization run in Windows.
- A hex editor is handy.  On Windows, I like [HxD](https://mh-nexus.de/en/hxd/).
- Real hardware that is USB bootable can be fun to put MOSS on.  After building, run
`source/x86/scripts/_install_usb.sh`.



## Changing Disk Image Parameters

To change the configuration of the disk image, we have to recreate it from scratch.  First, create
an empty image of the desired size in the ".build/" directory.  E.g. the following produces a 64 MiB
disk:

    dd   if=/dev/zero of=.build/diskimage.img   bs=1048576 count=64

Now we have to format it:

    fdisk   .build/diskimage.img

Now input the following commands, adjusting the starting sector (2048) and partition size (+32M) as
desired.  Note that for FAT32, the minimum partition size is (sector size) ⨯ 65527 ≈ 32 MiB:

    g
    n p
    1
    2048
    +32M
    t 1
    1
    w

Respectively, this creates a new GPT partition table ("g"), creates a new primary partition ("n p"),
configures the partition ("1" is first partition, "2048" is starting sector 2048, "+32M" is 32 MiB),
changes the type of the partition to "EFI System" ("t 1" selects the first partition, "1" is the EFI
type), and then finally writes out the changes to the file ("w").  [(See also.)](https://wiki.osdev.org/Bootable_Disk#Creating_the_Partitioning_Table)

We can also quickly save and restore this partitioning using "sfdisk".  Saving:

    sfdisk   -d .build/diskimage.img   >   src/diskimage64M.cfg

Restoring (note that we need the disk image file from above if not already present):

    dd   if=/dev/zero of=.build/diskimage.img   bs=1048576 count=64
    sfdisk   .build/diskimage.img < src/diskimage64M.cfg



## Canned Regexen

`template\s?<(\S[^<>]+\S)>` -> `template< $1 >`
`(?<!int|_t |\*)const\s+(?!noexcept|volatile)(\w+)` -> `$1 const` (though many false positives)

`typedef (\S+)\s+(\S+);` -> `using $2 = $1;` in C++ files



## VirtualBox

Set (system level!) environment variable `VBOX_GUI_DBG_ENABLED` to `true` to
[enable debugging](https://www.virtualbox.org/manual/ch12.html#ts_debugger) before starting any
VirtualBox processes.  Now, when you start the VM, you can open "Debug -> Command Line...", which
attaches a control terminal.  Some useful commands:

    rg[ <regname>]   (prints the register, or all registers if unspecified)

    cpu[ <index>]    (set current CPU, or output if omitted)

    stop             (pause)
    g                (continue)

    kg               (print stack)

    u[ <addr>]       (unassemble at)

    dg               (print GDT)
    di               (print IDT)
    dl               (print LDT)
    dt               (print TSS)

    info cpuid       (print virtual CPUID)
    info mmio        (print MMIO)

    help commands
    help <command>

The single-stop debugging seems broken.  "t" is supposed to single-step, but it actually just
returns.  It doesn't implement multiple arguments the way its own documentation says.  Also, "stop"
can make VirtualBox hang when more than one core is enabled.
