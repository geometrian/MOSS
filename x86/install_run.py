from subprocess import call
import sys, time

from build_print_info import *

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
    
print_info("Installing bootloader")
install(4)
print_info()

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
time.sleep(1)
