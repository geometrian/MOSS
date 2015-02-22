from subprocess import call
import sys, time

from build_print_info import *
from which_nasm import *
from which_gcc import *
from which_gpp import *
from which_ld import *

objects = []

def assemble(file_path,out_name):
    call([
        nasm,
        "-f","elf32",
        file_path,
        "-o","../build/"+out_name
    ])
    objects.append(out_name)
def compile_kernel():
    print_info("Assembling jump")
    assemble("../MOSS/kernel/jump.asm","jump.o")

    print_info("Assembling kernel assembly routines")
    assemble("../MOSS/kernel/gdt_idt/gdt.asm","gdt_asm.o")
    assemble("../MOSS/kernel/gdt_idt/idt.asm","idt_asm.o")
    assemble("../MOSS/kernel/gdt_idt/isr.asm","isr_asm.o")
    
    print_info("Compiling kernel source")
    modules = ["ui/console","ui/graphics","kernel"]
    for module in modules:
        print "Compiling module",module
        obj = module.split("/")[-1] + ".o"
        call([
            gpp,
            "-c",
            "-Wall","-Wextra",#"-Werror",
            "-nostdlib","-fno-builtin","-nostartfiles","-nodefaultlibs","-nostdinc","-ffreestanding",
            "../MOSS/kernel/"+module+".cpp",
            "-o","../build/"+obj
        ])
        objects.append(obj)
    
    print_info("Linking")
    args = [ld,"-T","linker.ld","--entry","jump"]
    for obj in objects:
        args.append("../build/"+obj)
    args += ["-o","../build/jump_kernel.elf"]
    call(args)

    print_info("Converting to .bin")
    call([
        "objcopy",
        "-O","binary","../build/jump_kernel.elf",
        "../build/kernel.bin"
    ])

def main():
    print_info("Compiling kernel")
    
    compile_kernel()

    print_info("Complete!",1)
if __name__ == '__main__':
    main()
##    time.sleep(2)
    raw_input()
