from subprocess import call
import sys

run_after_build = True

def print_info(text="",extra_newlines=0):
    if text != "":
        print "############## "+text+" ##############"
    for i in xrange(extra_newlines): print
    sys.stdout.flush()

def assemble_compile_bootloader1():
    call([
        "C:\\dev\\nasm\\nasm.exe",
        "MOSS/bootloader1/bootloader1.asm",
        "-i","MOSS/bootloader1/",
        "-i","MOSS/asm_helpers/",
        "-f","bin",
        "-o","bootloader1.bin"
    ])
def assemble_bootloader2():
##    call(["C:\\dev\\nasm\\nasm.exe","-hf"])
    call([
        "C:\\dev\\nasm\\nasm.exe",
        "MOSS/bootloader2/bootloader2.asm",
        "-i","MOSS/bootloader2/",
        "-i","MOSS/asm_helpers/",
        "-f","elf",
        "-o","bootloader2.o"
    ])
##def assemble_kernel():
##    call([
##        "C:\\dev\\Perl\\c\\bin\\gcc.exe",
##        "-S",
##        "MOSS/kernel/kernel.c",
##        "-o","__temp_kernel__.asm",
##        "-Wall","-Wextra","-Werror",
##        "-nostdlib","-fno-builtin","-nostartfiles","-nodefaultlibs"
##    ])
##    call([
##        "C:\\dev\\nasm\\nasm.exe",
##        "__temp_kernel__.asm",
##        "-i","MOSS/bootloader2/",
##        "-i","MOSS/asm_helpers/",
##        "-i","MOSS/kernel/",
##        "-f","elf",
##        "-o","kernel.o"
##    ])
##def compile_kernel():
##    call([
##        "C:\\dev\\Perl\\c\\bin\\gcc.exe",
##        "-c",
##        "MOSS/kernel/kernel.c",
##        "-o","kernel.o",
##        "-Wall","-Wextra","-Werror",
##        "-nostdlib","-fno-builtin","-nostartfiles","-nodefaultlibs"
##    ])
##def link_bootloader2_kernel():
##    call([
##        "C:\\dev\\Perl\\c\\bin\\ld.exe",
##        "-T","kernel_link.ld",
##        "-o","kernel.bin",
##        "bootloader2.o"#,"kernel.o"
##    ])
####    call([
####        "../0.0/VC152/LINK.EXE",
####        "bootloader2.o","kernel.o",
####        "-o","kernel.bin"
####    ])
    
def install(mb): #Copy bootloader into hard drive
    f = open("bootloader1.bin","rb")
    data1 = f.read()
    f.close()

    f = open("kernel.bin","rb")
    data2 = f.read()
    f.close()

    data = data1 + data2

    data += "\x00"*(1*1024*1024)

    f = open("C:\\Users\\Ian Mallett\\VirtualBox VMs\\MOSS\\MOSS-flat.vmdk","wb")
    f.write(data)
    f.close()

print_info("Build begin")

print_info("Assembling/Compiling bootloader 1")
assemble_compile_bootloader1()

print_info("Assembling bootloader 2")
assemble_bootloader2()

##print_info("Compiling Kernel")
##compile_kernel()
##
##print_info("Linking Bootloader 2 and Kernel")
##link_bootloader2_kernel()

print_info("Build complete!",1)

##sys.exit()

if run_after_build:
    print_info("Installing bootloader")
    install(4)
    print_info()

##    print_info("Configuring virtual machine")
##    call([
##        "C:\\Program Files\\Oracle\\VirtualBox\\VBoxManage.exe","modifyvm",
##        "MOSS",
##        "--bioslogofadein","off",
##        "--bioslogodisplaytime","0",
##        "--bioslogofadeout","off",
##        "--biosbootmenu","disabled"
##    ])

    sys.exit()
    
    print_info("Booting virtual machine")
    call([
        "C:\\Program Files\\Oracle\\VirtualBox\\VBoxManage.exe",
        "startvm","MOSS"
    ])

    print_info("Bringing to front")
    call([
        "nircmd\\nircmd.exe","win","activate","title","MOSS [Running] - Oracle VM VirtualBox"
    ])

    print_info("Virtual machine started")
