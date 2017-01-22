from subprocess import call


def main():
##    #VirtualBox has a weird thing where it crashes if the UUID of the new hard drive is not what it expects.
##    #The line in _install_vb makes the UUID all zeros, apparently, but we can fix that by removing the disk
##    #and then readding it.  See https://forums.virtualbox.org/viewtopic.php?p=33678
##    call([
##        "C:\\Program Files\\Oracle\\VirtualBox\\VBoxManage.exe",
##        "modifyvm","MOSS",
##        "-hda","none"
##    ])
##    input()
##    call([
##        "C:\\Program Files\\Oracle\\VirtualBox\\VBoxManage.exe",
##        "unregisterimage","disk","../VM/MOSS-disk-flat.vmdk"
##    ])
##    input()
##    call([
##        "C:\\Program Files\\Oracle\\VirtualBox\\VBoxManage.exe",
##        "modifyvm","MOSS",
##        "-hda","../VM/MOSS-disk-flat.vmdk"
##    ])
##    input()
    call([
        "C:\\Program Files\\Oracle\\VirtualBox\\VBoxManage.exe",
        "internalcommands","sethduuid","C:\\dev\\C++\\MOSS\\VMs\\MOSS-disk-flat.vmdk","3e35d8c1-f057-4b4c-9d9e-120881738019"
    ])
    
    call([
        "C:\\Program Files\\Oracle\\VirtualBox\\VBoxManage.exe",
        "startvm","MOSS"
    ])

    call([
        "..\\..\\resources\\nircmd\\nircmd.exe","win","activate","title","MOSS [Running] - Oracle VM VirtualBox"
    ])
if __name__ == "__main__": main()
