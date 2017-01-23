# MOSS
## The Minimal Operating System That Sucks

Once, in college, I woke up in the middle of the night and realized I had to write an operating
system.  No delusions, mind, it would almost certainly remain a hobbyist endeavor.  I'd be happy
if I could get anything working.

Since then, MOSS has matured, but still lives up to its acronym: the Minimal Operating System
that Sucks.

## Orientation

A few selected locations in the file structure to help you get your bearings.

- `cross/` is a handy location to put your cross-compiler.  You need a cross-compiler, and building
it [is also documented there](cross/README.md).
- `resources/` is a place for content-creation (fonts, images, files, associated scripts, etc.).  A
bit messy, I'm afraid.
- `source/x86/` is where MOSS's source is kept, along with project files and build directories.
The ARM branch is not currently being developed.
  - `source/x86/.build/` is where MOSS disk images get built.
  - `source/x86/files/` is where you can add files to be included in the built disk image.
  - `source/x86/scripts/` are helper scripts used to build and convert MOSS images.  Most are messy
and many will require some modifications to paths to work on others' machines.  This should be
fixed.
  - `source/x86/source_*` sources for MOSS and standard libraries.
  - `source/x86/_run_*_win.py` are scripts that conveniently spool up a built MOSS disk image in an
emulator.  Will likely require modifications to paths to work on others' machines.  This should be
fixed.
- `VMs/` is a handy location to put virtual machines for emulation, and is also the location to
which emulator builds output.

## Requirements

- [Python](https://www.python.org/), for a lot of scripts.
- Linux-y environment (I use an x86-64 Ubuntu VM), with ability to install packages.
- [Visual Studio](https://www.visualstudio.com/) is recommended for the project files.
- An emulator, such as [Bochs](http://bochs.sourceforge.net/) or
[VirtualBox](https://www.virtualbox.org/wiki/VirtualBox), or real hardware that is USB bootable.

Because (the real version of) Visual Studio currently only runs on Windows, the recommended setup
is a Windows machine, with a shared file structure to a Linux VM.  Do development in Visual Studio;
do compiles in the VM.

## Philosophy

MOSS is designed to be as powerful, but still as clear as possible.  In particular, the main
language is C++, not C, which allows most of the horrible hardware crap to be abstracted into
semi-OO interfaces.  Not all code is up to this standard, since a lot of it was written during
various 04:00 impromptu hackathons, and parts of it are being rewritten, even as the functionality
improves.

## Contributing

MOSS is free for use with attribution, and your pull requests are welcome.  Unfortunately, I
haven't had time to set up a reproducible way of setting things up, and so, like much else in
OS development, it is likely to be painful.
