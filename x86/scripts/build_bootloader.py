from subprocess import call
import sys, time

from build_print_info import *
from which_nasm import *

def assemble_compile_bootloader1():
    call([
        nasm,
        "../MOSS/bootloader/bootloader.asm",
        "-i","../MOSS/bootloader/",
        "-i","../MOSS/asm_helpers/",
        "-f","bin",
        "-o","../build/bootloader.bin"
    ])

def main():
    print_info("Assembling/Compiling bootloader")
    assemble_compile_bootloader1()

    print_info("Complete!",1)
if __name__ == '__main__':
    main()
    time.sleep(2)
