import os
from subprocess import call

from scripts import _paths


def main():
##    #VirtualBox has a weird thing where it crashes if the UUID of the new hard drive is not what it expects.
##    #The line in _install_vb makes the UUID all zeros, apparently, but we can fix that by removing the disk
##    #and then readding it.  See https://forums.virtualbox.org/viewtopic.php?p=33678
##    call([
##        _paths.get_virtualbox_manage(),
##        "modifyvm","MOSS",
##        "-hda","none"
##    ])
##    input()
##    call([
##        _paths.get_virtualbox_manage(),
##        "unregisterimage","disk","../VM/MOSS-disk-flat.vmdk"
##    ])
##    input()
##    call([
##        _paths.get_virtualbox_manage(),
##        "modifyvm","MOSS",
##        "-hda","../VM/MOSS-disk-flat.vmdk"
##    ])
##    input()
    call([
        _paths.virtualbox_manage,
        "internalcommands","sethduuid",_paths.vmdk,"3e35d8c1-f057-4b4c-9d9e-120881738019"
    ])
    
    call([
        _paths.virtualbox_manage,
        "startvm","MOSS"
    ])

    if os.name == "nt":
        call([
            _paths.nircmd,"win","activate","title","MOSS [Running] - Oracle VM VirtualBox"
        ])
if __name__ == "__main__": main()
