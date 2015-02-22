from subprocess import call
import sys, time

from build_print_info import *

def assemble_compile_bootloader1():
    call([
        "C:\\dev\\nasm\\nasm.exe",
##        "nasm"
        "MOSS/bootloader1/bootloader1.asm",
        "-i","MOSS/bootloader1/",
        "-i","MOSS/asm_helpers/",
        "-f","bin",
        "-o","bootloader1.bin"
    ])

def main():
    print_info("Assembling/Compiling bootloader 1")
    assemble_compile_bootloader1()

    print_info("Complete!")
if __name__ == '__main__':
    main()
    time.sleep(2)
