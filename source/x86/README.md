# MOSS Source Directory

## Building

Builds happen in a Linux environment.  There are several stages to building:

1. (Re)compile and (re)link MOSS itself (`./scripts/build.sh`).  This is the most used script when
developing.
2. (Re)create a raw virtual disk (`./scripts/release_setup.sh`).  After the first run, the disk is
created and formatted.  To improve performance on later runs, set the `remake_*` variables to `0`
in that script (around line 30).
3. Copy the raw virtual disk to formatted virtual disk(s) and/or a USB stick
(`./scripts/install.sh`; by default, copies only to a formatted virtual disk of type VMDK).

Do all of the above: (`./scripts/all.sh`).

## Running

MOSS runs in a variety of virtualization environments as well as on USB-bootable hardware.
[For more about VM environments supported by MOSS](../../VMs/README.md).

- `./_run_bochs.py`: Runs an already-built MOSS image on the system's Bochs emulator.
- `./_run_bochsdbg.py`: Runs an already-built MOSS image on the system's Bochs emulator, using
debug parameters.
- `./_run_qemu.py`: Runs an already-built MOSS image on the system's QEMU emulator.
- `./_run_virtualbox.py`: Runs an already-built MOSS image on the system's VirtualBox emulator.  A
VirtualBox VM must have already been created for MOSS (see the VM environments link above).
