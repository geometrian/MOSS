from subprocess import call
import sys, time

from build_print_info import *

def main():
    print_info("Booting VirtualBox virtual machine")
    call([
##        "C:\\Program Files\\Oracle\\VirtualBox\\VBoxManage.exe",
        "VBoxManage",
        "startvm","MOSS"
    ])

##    print_info("Bringing to front")
##    call([
##        "nircmd\\nircmd.exe","win","activate","title","MOSS [Running] - Oracle VM VirtualBox"
##    ])

    print_info("Virtual machine started",1)
if __name__ == '__main__':
    main()
    time.sleep(2)
