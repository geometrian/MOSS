from subprocess import call
import sys, time

from build_print_info import *

print_info("Configuring virtual machine")
call([
##    "C:\\Program Files\\Oracle\\VirtualBox\\VBoxManage.exe",
    "VBoxManage",
    "modifyvm",
    "MOSS",
    "--bioslogofadein","off",
    "--bioslogodisplaytime","0",
    "--bioslogofadeout","off",
    "--biosbootmenu","disabled"
])

print_info("Complete!",1)
time.sleep(1)
