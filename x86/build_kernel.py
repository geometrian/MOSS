from subprocess import call
import sys, time

from build_print_info import *
from which_nasm import *
from which_gcc import *
from which_ld import *

##def compile_kernel():
##    call([
####        "gcc",
##        "C:\\dev\\Perl\\c\\bin\\gcc.exe",
##        "-c",
####        "-m32",
##        "-Wall","-Wextra","-Werror",
##        "-nostdlib","-fno-builtin","-nostartfiles","-nodefaultlibs",
####        "--entry","kernel_main",
##        "MOSS/kernel/kernel.c",
##        "-o","kernel.bin"
##    ])
##def compile_kernel():
##    call([
##        "C:\\dev\\nasm\\nasm.exe",
##        "MOSS/kernel/jump.asm",
##        "-f","bin",
##        "-o","kernel.bin"
##    ])
def compile_kernel():
    print_info("Assembling jump")
    #nasm  -f elf32  MOSS/kernel/jump.asm  -o jump.o
    call([
        nasm,
        "-f","elf32",
        "MOSS/kernel/jump.asm",
        "-o","jump.o"
    ])
    
    print_info("Compiling kernel")
    #gcc  -c  -Wall -Wextra -Werror  -nostdlib -fno-builtin -nostartfiles -nodefaultlibs -nostdinc -ffreestanding  MOSS/kernel/kernel.c  -o kernel.o
    call([
        gcc,
        "-c",
##        "-m32","-march=i686",#"-march=i386","-b i386",
        "-Wall","-Wextra",#"-Werror",
        "-nostdlib","-fno-builtin","-nostartfiles","-nodefaultlibs","-nostdinc","-ffreestanding",
        "MOSS/kernel/kernel.c",
        "-o","kernel.o"
    ])

##    print_info("Linking")
####    #ld -m elf_i386 -s -r --oformat binary --entry jump jump.o kernel.o -o kernel.bin
####    call([
####        ld,
######        "-m","i386pep",
######        "-m","i386pe",
####        "-m","elf_i386",
####        "-s",
####        "-r",
######        "--oformat","binary",
######        "--oformat=elf32-i386",
####        "--entry","jump",
####        "jump.o",#"kernel.o",
####        "-o","kernel.bin"
####    ])
##    call([
##        ld,
####        "-T","linker.ld",
##        "--oformat","binary",
##        "--entry","jump",
##        "jump.o","kernel.o",
##        "-o","kernel.bin"
##    ])
    
    print_info("Linking")
    #ld  --entry jump  jump.o kernel.o  -o jump_kernel.elf
    call([
        ld,
##        "-Ttext","0x1000",
        "-T","linker.ld",
        "--entry","jump",
        "jump.o","kernel.o",
        "-o","jump_kernel.elf"
    ])
    print_info("Converting to .bin")
    #objcopy  -O binary  jump_kernel.elf  kernel.bin
    call([
        "objcopy",
##        "-I","elf32",
        "-O","binary","jump_kernel.elf",
        "kernel.bin"
    ])

def main():
    print_info("Compiling kernel")
    
    compile_kernel()

    print_info("Complete!",1)
if __name__ == '__main__':
    main()
##    time.sleep(2)
    raw_input()
