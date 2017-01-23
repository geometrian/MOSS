# Cross Compiler

To build MOSS (or any non-self-hosting OS), you need a cross-compiler.  GCC is supported, and this
file gives instructions on how to build it.  You will need a Linux environment (I use an x86-64
Ubuntu VM).

# Step 0: Install Prerequisites

To build GCC, we need some prerequisite libraries.  Install
[Bison](https://www.gnu.org/software/bison/), [flex](https://github.com/westes/flex),
[`GMP`](https://gmplib.org/), [`ISL`](http://isl.gforge.inria.fr/),
[`MPC`](http://multiprecision.org/), [`MPFR`](http://www.mpfr.org/), and
[`texinfo`](https://www.gnu.org/software/texinfo/) onto your system.  ISL is technically optional,
but desirable since it enables some optimizations.  Others may be optional or tweakable too.  But,
on `apt` systems, all this can be simply resolved with:

    sudo apt-get install bison flex libgmp-dev libmpfr-dev libisl-dev libmpc-dev texinfo

If you're not on `apt`, it should be similar.  Else, you can check for instructions on
[OSDev](http://wiki.osdev.org/GCC_Cross-Compiler#Downloading_the_Source_Code), or install from
source:

- [Bison](ftp://ftp.gnu.org/gnu/bison/)
- [flex](https://github.com/westes/flex/releases)
- [GMP](ftp://ftp.gnu.org/gnu/gmp/)
- [ISL](ftp://gcc.gnu.org/pub/gcc/infrastructure/) (note: potentially more up-to-date releases can be found [here](http://isl.gforge.inria.fr/))
- [MPC](ftp://ftp.gnu.org/gnu/mpc/)
- [MPFR](ftp://ftp.gnu.org/gnu/mpfr/)
- [texinfo](ftp://ftp.gnu.org/gnu/texinfo/)

# Step 1: Setup Build Directory

Create a directory for the builds (I'll call it `cross/` in this tutorial).  It may be possible to
move this directory later, but note that for now it must be on a filesystem that supports hardlinks
in a Linux-y (i.e. deliberately-non-portable shell-script-y) way (the compile uses this internally
for some asinine reason).  Also export this path for future use.  For me, in my VM, I just put it
in my home directory:

    export PREFIX=~/cross/
    #export PREFIX=/media/sf_dev/C++/MOSS/cross/
    mkdir $PREFIX


# Step 2: Downloads

Download the following and extract them as folders into `cross/`:

- [binutils](ftp://ftp.gnu.org/gnu/binutils/)
- [GCC](ftp://ftp.gnu.org/gnu/gcc/)

One should use the latest versions of these packages.  But here are some tested, known-good
configurations:

| Packages   | 2017-01-22 |
|:----------:|:-----------|
| binutils   | 2.27       |
| gcc        | 6.3.0      |

#| gmp        | 6.1.2      |
#| isl        | 0.18       |
#| mpc        | 1.0.3      |
#| mpfr       | 3.1.5      |
#| texinfo    | 6.3        |

# Step 3: Set Up Build

Export some version numbers just for later convenience (make sure yours match the ones you
downloaded; these match those of the latest tested build, above).  Also export the target
architecture (necessary for a freestanding OS) and adjust the shell's copy of `PATH` (necessary to
resolve dependencies with the downloaded/built ones, instead of different ones existing on system):

    export VER_BINUTILS=2.27
    export VER_GCC=6.3.0
    #export VER_GMP=6.1.2
    #export VER_ISL=0.18
    #export VER_MPC=1.0.3
    #export VER_MPFR=3.1.5
    #export VER_TEXINFO=6.3

    export TARGET=i686-elf
    export PATH="$PREFIX/bin:$PATH"

#Run the following commands (copies the downloaded dependencies into the `binutils` and `gcc`
#directories, where they will be built automatically when needed):

    #cd $PREFIX
    #mv gmp-$VER_GMP gcc-$VER_GCC/gmp
    #mv isl-$VER_ISL gcc-$VER_GCC/isl
    #mv mpc-$VER_MPC gcc-$VER_GCC/mpc
    #mv mpfr-$VER_MPFR gcc-$VER_GCC/mpfr
    #mv texinfo-$VER_TEXINFO binutils-$VER_BINUTILS/texinfo

# Step 4: Set Up And Build `binutils`

Run the following commands to configure and build cross-compile `binutils`.
#The `echo` [effectively removes](http://wiki.osdev.org/Talk:GCC_Cross-Compiler#Texinfo) a dependency on
#the package `texinfo`; if you have `texinfo` installed, it shouldn't make a difference.

    cd $PREFIX
    mkdir build-binutils
    cd build-binutils
    ../binutils-$VER_BINUTILS/configure --target=$TARGET --prefix=$PREFIX --disable-nls
    #echo "MAKEINFO = :" >> Makefile
    make
    make install
    cd ..

The build will take a bit of time.

At this point, you should have a bunch of binaries built into `cross/bin/`.  In particular, you
should have e.g. `i686-elf-as`, `i686-elf-ld`, etc.

# Step 5: Set Up And Build `GCC`

Run the following commands to configure and build a cross-compile `GCC`.  Note: during the
configure, it will say "This configuration is not supported in the following subdirectories:", and
then list some.  That's expected and fine.  Also, a lot of the paths found should be to places in
`cross/i686-elf/bin/`.

    cd $PREFIX
    mkdir build-gcc
    cd build-gcc
    ../gcc-$VER_GCC/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers
    make all-gcc
    make all-target-libgcc
    make install-gcc
    make install-target-libgcc

The build will take a relatively long time (on the order of an hour or more).

At this point, you should have a successful cross-compiler (i.e., `i686-elf-gcc`/`i686-elf-g++`)
built into `cross/bin/`!







export VER_BINUTILS=2.27
export VER_GCC=6.3.0
export VER_GMP=6.1.2
export VER_ISL=0.16.1
export VER_MPC=1.0.3
export VER_MPFR=3.1.5
export VER_TEXINFO=6.3

export PREFIX=/media/sf_dev/C++/MOSS/cross/
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"

cd $PREFIX

mv gmp-$VER_GMP gcc-$VER_GCC/gmp
mv isl-$VER_ISL gcc-$VER_GCC/isl
mv mpc-$VER_MPC gcc-$VER_GCC/mpc
mv mpfr-$VER_MPFR gcc-$VER_GCC/mpfr

mkdir build-binutils
cd build-binutils
../binutils-$VER_BINUTILS/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
make
make install
cd ..

# The $PREFIX/bin dir _must_ be in the PATH. We did that above.
which -- $TARGET-as || echo $TARGET-as is not in the PATH

mkdir build-gcc
cd build-gcc
../gcc-$VER_GCC/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers
make all-gcc
make all-target-libgcc
make install-gcc
make install-target-libgcc
























