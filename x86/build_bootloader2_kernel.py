from subprocess import call
import sys, time

from build_print_info import *

def assemble_bootloader2():
    call([
        "C:\\dev\\nasm\\nasm.exe",
##        "nasm"
        "MOSS/bootloader2/bootloader2.asm",
        "-i","MOSS/bootloader2/",
        "-i","MOSS/asm_helpers/",
        "-f","elf",
        "-o","bootloader2.o"
    ])

def main():
    print_info("Assembling bootloader 2")
    assemble_bootloader2()

    print_info("Complete!")

    print "Now run \"build_link_kernel.py\" in a Linux VM."
if __name__ == '__main__':
    main()
    time.sleep(5)
