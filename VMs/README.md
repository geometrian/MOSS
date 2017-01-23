#Virtual Machines

This directory is a handy location for (and hence the default location of) emulators' builds.
For emulators, MOSS supports being build for [Bochs](http://bochs.sourceforge.net/) and for
[VirtualBox](https://www.virtualbox.org/wiki/VirtualBox).

One thing you should be aware of is that, for performance, some virtualization software on uses a
hardware feature called `VT-x`.  This is a virtualization bit, and can't be used by more than one
application at a time.  For example, VirtualPC grabs VT-x and doesn't share.  Therefore,
[it breaks VirtualBox](https://forums.virtualbox.org/viewtopic.php?f=6&t=64624) if you try to run
VirtualBox at the same time.

## [QEMU](http://wiki.qemu.org/Main_Page)

A QEMU utility is used for converting disk images for VirtualBox (i.e., to `.vmdk`), and so is its
prerequisite.

## VirtualBox

MOSS runs well here.

## Bochs

Bochs is good, but very slow.  It doesn't need anything except the raw `.bin` kernel image.

## VMWare Player

Not tested recently.

## VirtualPC

Not tested recently.
