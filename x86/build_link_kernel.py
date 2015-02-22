from subprocess import call
import sys, time

from build_print_info import *

def compile_kernel():
    call([
        "gcc",
        "-m32",
        "-c",
        "-Wall","-Wextra","-Werror",
        "-nostdlib","-fno-builtin","-nostartfiles","-nodefaultlibs",
        "MOSS/kernel/kernel.c",
        "-o","kernel.o"
    ])
def link_bootloader2_kernel():
    call([
        "ld",
        "-s",
        "--entry","loader",
        "-m","elf_i386",
        "bootloader2.o","kernel.o",
        "-o","kernel.bin"
    ])

def main():
    print_info("Compiling kernel")
    compile_kernel()

    print_info("Linking kernel and second stage bootloader")
    link_bootloader2_kernel()

    print_info("Complete!")

    print "Now run \"install_run.py\" to install and run MOSS"
if __name__ == '__main__':
    main()
    time.sleep(5)
