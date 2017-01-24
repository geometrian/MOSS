# Cross Compiler

To build MOSS (or any other OS), you need a cross-compiler.  This is because an ordinary compiler
assumes that there's a platform to run on--something that obviously doesn't exist when you're
trying to compile that platform.

MOSS supports `GCC`, and this file gives instructions on how to build a `GCC` cross-compiler.  You
will need a Linux environment (I use an x86-64 Ubuntu VM).

## Step 0: Install Prerequisites

To build GCC, we need some prerequisite libraries.  Install
[`Bison`](https://www.gnu.org/software/bison/), [`flex`](https://github.com/westes/flex),
[`GMP`](https://gmplib.org/), [`ISL`](http://isl.gforge.inria.fr/),
[`MPC`](http://multiprecision.org/), [`MPFR`](http://www.mpfr.org/), and
[`texinfo`](https://www.gnu.org/software/texinfo/) onto your system.  (`ISL` is technically
optional, but is desirable since it enables some optimizations.  Others may be optional or
tweakable too.)  On `apt` systems, all this can be simply accomplished with:

    sudo apt-get install bison flex libgmp-dev libmpfr-dev libisl-dev libmpc-dev texinfo

If you're not on `apt`, it should be similar.  Else, you can check for instructions on
[OSDev](http://wiki.osdev.org/GCC_Cross-Compiler#Downloading_the_Source_Code), or install from
sources.  Sources' links:

- [Bison][1]
- [flex][2]
- [GMP][3]
- [ISL][4] (note: potentially more up-to-date releases can be found
[here](http://isl.gforge.inria.fr/))
- [MPC][5]
- [MPFR][6]
- [texinfo][7]

## Step 1: Setup Build Directory

Create a directory for the builds (I'll call it `cross/` in this tutorial).  It may be possible to
move this directory later, but note that for now it must be on a filesystem that supports hardlinks
in a Linux-y (i.e. deliberately-non-portable shell-script-y) way (the compile uses this internally
for some asinine reason).  Also export this path for future use.  For me, in my VM, I just put it
in my home directory, and the whole operation becomes:

    export PREFIX=~/cross/
    mkdir $PREFIX

If you want to use a different directory, you should also change the value in
`source/x86/scripts/_paths.py`.

## Step 2: Downloads

Download sources for `binutils` and `GCC` and extract them as folders into `cross/`.  Sources'
links:

- [binutils](ftp://ftp.gnu.org/gnu/binutils/)
- [GCC](ftp://ftp.gnu.org/gnu/gcc/)

Almost certainly, one should use the latest versions of these.  But here are some tested,
known-good configurations:

| Packages   | 2017-01-22                                                     |
|:----------:|:--------------------------------------------------------------:|
| binutils   | [2.27](ftp://ftp.gnu.org/gnu/binutils/binutils-2.27.tar.bz2)   |
| gcc        | [6.3.0](ftp://ftp.gnu.org/gnu/gcc/gcc-6.3.0/gcc-6.3.0.tar.bz2) |

## Step 3: Set Up Build

Export some version numbers just for later convenience (make sure yours match the ones you
downloaded; these match those of the latest tested build, above).  Also export the target
architecture and adjust the shell's copy of `PATH` (necessary to resolve `GCC`'s dependencies using
the downloaded/built `binutils` ones, instead of different ones potentially preexisting on the
system).

    export VER_BINUTILS=2.27
    export VER_GCC=6.3.0

    export TARGET=i686-elf
    export PATH="$PREFIX/bin:$PATH"

## Step 4: Set Up And Build `binutils`

Run the following commands to configure and build cross-compile-`binutils`:

    cd $PREFIX
    mkdir build-binutils
    cd build-binutils
    ../binutils-$VER_BINUTILS/configure --target=$TARGET --prefix=$PREFIX --disable-nls
    make
    make install
    cd ..

The build will only take a couple minutes on a newish computer.

At this point, you should have a bunch of binaries built into `cross/bin/`.  In particular, you
should have e.g. `i686-elf-as`, `i686-elf-ld`, etc.  I recommend you verify this up-front, as the
correctness of the long `GCC` build depends on it, and it seems that problems do occur here
especially.

If you do get a problem, it's probably reported and described in the output of the configuration
step.  Let me know about it, since it might be preventable.

## Step 5: Set Up And Build `GCC`

Run the following commands to configure and build a cross-compile-`GCC`.  Note: during the
configure, it will say "This configuration is not supported in the following subdirectories:", and
then list some.  That's expected and fine.  Also, a good sign is if a lot of the paths found are to
places in `cross/i686-elf/bin/`.

    cd $PREFIX
    mkdir build-gcc
    cd build-gcc
    ../gcc-$VER_GCC/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers
    make all-gcc
    make all-target-libgcc
    make install-gcc
    make install-target-libgcc

The build will take a relatively long time (on the order of an hour or more).

At this point, you should have a cross-compiler (i.e., `i686-elf-gcc`/`i686-elf-g++`) built into
`cross/bin/`!

   [1]: ftp://ftp.gnu.org/gnu/bison/
   [2]: https://github.com/westes/flex/releases
   [3]: ftp://ftp.gnu.org/gnu/gmp/
   [4]: ftp://gcc.gnu.org/pub/gcc/infrastructure/
   [5]: ftp://ftp.gnu.org/gnu/mpc/
   [6]: ftp://ftp.gnu.org/gnu/mpfr/
   [7]: ftp://ftp.gnu.org/gnu/texinfo/