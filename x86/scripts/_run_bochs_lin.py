from subprocess import call
import sys, time

from build_print_info import *

def main():
    print_info("Booting Bochs virtual machine")
    #http://bochs.sourceforge.net/doc/docbook/user/using-bochs.html#COMMANDLINE
##    call([
##        "C:\\Program Files (x86)\\Bochs-2.6\\bochs.exe",
##        "-q",
##        "-f","C:/dev/C++/MOSS/0.4/bochs-MOSS-win.txt"
##    ])
    call([
        "bochs",
        "-q",
        "-f","../bochs-MOSS-lin.txt"
    ])

##    print_info("Bringing to front")
##    call([
##        "nircmd\\nircmd.exe","win","activate","title","MOSS [Running] - Oracle VM VirtualBox"
##    ])

    print_info("Virtual machine started",1)
if __name__ == '__main__':
    main()
##    time.sleep(2)
