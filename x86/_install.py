from subprocess import call
import sys, time

from build_print_info import *

def install(mb): #Copy bootloader into hard drive
    print_info("Installing bootloader")
    
    f = open("bootloader.bin","rb")
    data1 = f.read()
    f.close()

    print_info("Installing kernel")
    f = open("kernel.bin","rb")
    data2 = f.read()
    f.close()

    data = data1 + data2
    data += "\x00"*(1*1024*1024-len(data))

    print_info("Writing data")
    f = open("disk_img.bin","wb")
    f.write(data)
    f.close()

##    f = open("C:\\Users\\Ian Mallett\\VirtualBox VMs\\MOSS\\MOSS-flat.vmdk","wb")
##    f = open("../../../../Users/Ian Mallett/VirtualBox VMs/MOSS/MOSS-flat.vmdk","wb")
    f = open("../VM/MOSS-flat.vmdk","wb")
    f.write(data)
    f.close()

    print_info("Complete!",1)

def main():
    install(4)
if __name__ == '__main__':
    main()
    time.sleep(2)
