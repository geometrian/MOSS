# MOSS



## The Minimal Operating System That Sucks

Once upon a time, in undergrad, I woke up in the middle of the night and realized I had to write an
operating system.  No delusions, mind—it would almost certainly remain a hobbyist endeavor.  I'd be
happy if I could get anything working.

<!-- https://code.google.com/archive/p/ianmallett-moss/ -->
The initial result first became an open-source project on Google Code, and since then, MOSS has
matured still further.  But still lives up to its acronym: the **M**inimal **O**perating **S**ystem
that **S**ucks.



## Orientation / Source Tree Overview

A few selected locations in the file structure to help you get your bearings.

- `doc/` is where all the more detailed documentation and hardware notes goes.
- `src/data/` is a place for developing assets related to MOSS.  For example hardware tables, fonts,
and logos.
- `src/kernel/` the source for the actual MOSS kernel.
- `src/libmoss-stdc/` a freestanding C standard library for use by the kernel.
- `src/libmoss-util/` utility code for low-level development.
- `src/root_fs/` the loaded contents of a MOSS disk image.
- `src/tests/` tests of certain functionality intended for checking on a development machine.



## Design Philosophy

MOSS is designed for clarity.  The x86 architecture in particular is extremely crufty, and I believe
it is vital to understand what it's doing more than making a shaky foundation do more stuff.

Language-wise, we use modern C++ whenever possible, and modern C when it is not.  Where applicable,
code clarity is preferred over minor improvements in code performance, and code overall is written
for clarity first.

Not all code is currently up to this standard, since a lot of it was written during various 04:00
impromptu hackathons, and parts of it are being rewritten, even as the functionality improves.



## Help and Contributions

MOSS's source is free for use with attribution, and your pull requests are welcome.  If you have
trouble setting up MOSS, contact me via the info on my website,
[geometrian.com](https://geometrian.com).
