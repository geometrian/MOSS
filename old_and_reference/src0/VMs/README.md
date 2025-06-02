# Virtual Machines

This directory is a handy location for (and hence the default location of) emulators' builds.
For emulators, MOSS officially supports being built for [Bochs](http://bochs.sourceforge.net/),
[QEMU](http://wiki.qemu.org/Main_Page), and
[VirtualBox](https://www.virtualbox.org/wiki/VirtualBox).

One thing you should be aware of is that, for performance, some (and notably not all)
virtualization software uses a hardware feature called `VT-x`.  This is a so-called "virtualization
bit", and it can't be used by more than one such application at a time.

Note that to enable this virtualization support in the first place (in particular for 64-bit
guests), it may be a BIOS / UEFI setting (see e.g. VT-x on Intel processors).



## [VirtualBox](https://www.virtualbox.org/wiki/VirtualBox)

MOSS runs well here.  Recommended.

### Setup (Linux):

1. Install and run VirtualBox: `sudo apt-get install virtualbox; virtualbox &`
2. Create a new VM (name: `MOSS`, Type: `Other`, Version: `Other/Unknown (64-bit)`, Enable EFI).  It
doesn't need much RAM.  64MB should be plenty.  Use an existing virtual hard disk file, and point it
to the disk the build created: `VMs/MOSS-disk-flat.vmdk`.
3. Start the VM (click on it or run `source/x86/_run_virtualbox.py`)

### Setup (Windows):

Same as for Linux, except in the first step, you'll have to download VirtualBox manually; e.g. from
[here](https://www.virtualbox.org/wiki/Downloads).












## [Bochs](http://bochs.sourceforge.net/)

Bochs is good, but somewhat slow.  MOSS runs well here.  Recommended.  Bochs also doesn't need
anything else except MOSS's raw `.bin` kernel image.



## [QEMU](http://wiki.qemu.org/Main_Page)

MOSS runs well here, and seems to start especially quickly.  The `qemu-img` utility is also used
for converting disk images for VirtualBox and VMware (i.e., to `.vmdk`), and so is their
prerequisite.



## Virtual PC

Not tested recently.  Successor is Hyper-V.



## [Hyper-V](https://docs.microsoft.com/en-us/virtualization/hyper-v-on-windows/quick-start/enable-hyper-v)

Not tested.  Only available on Windows 10 Pro.  Using this will grab the VT-x bit and
[prevent other virtualization software from using it](https://forums.virtualbox.org/viewtopic.php?f=6&t=64624).
Unfortunately, there's no way around this.  To disable Hyper-V again, Start -> Run ->
OptionalFeatures.exe -> Uncheck VT-x.



## [VMware[ Workstation][ Player]](http://www.vmware.com/)

Not tested recently; ever since version 7, the free version requires subscribing to advertising
emails.  This is still true as of version 12.
