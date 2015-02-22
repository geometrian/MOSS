from subprocess import Popen
import sys, time

from build_print_info import *

def main():
    print_info("Starting VM")
    Popen([
        "C:\\Program Files\\Oracle\\VirtualBox\\VirtualBox.exe",
        "--dbg",
        "--startvm","MOSS"
    ])

    #http://wiki.yobi.be/wiki/RAM_analysis
    print_info("Dump RAM")
    print "Click on the \"Debug\" menu -> \"Command line...\""
    print ".pgmphystofile <filename>"
    raw_input("RETURN to exit")
    
    print_info("Complete!")
if __name__ == '__main__':
    main()
##    time.sleep(2)
