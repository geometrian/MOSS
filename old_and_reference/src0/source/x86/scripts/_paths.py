#Various paths used by the build system.  Multiple paths can be used, so that multiple environments
#   can be supported.

import os

#User may need to change some of these, particularly for Windows.
#=========================================================================
if os.name == "nt":
    #Windows Paths
    bochs = "C:/Program Files (x86)/Bochs-2.6.8/bochs.exe"
    bochsdbg = "C:/Program Files (x86)/Bochs-2.6.8/bochsdbg.exe"
    virtualbox_manage = "C:/Program Files/Oracle/VirtualBox/VBoxManage.exe"
    qemu32 = "C:/Program Files/qemu/qemu-system-i386.exe"
    qemu64 = "C:/Program Files/qemu/qemu-system-x86_64.exe"
    nircmd = "C:/Program Files (x86)/nircmd/nircmd.exe"
else:
    #Non-Windows Paths
    from distutils import spawn
    bochs = spawn.find_executable("bochs")
    bochsdbg = bochs
    virtualbox_manage = spawn.find_executable("VBoxManage")
    qemu32 = spawn.find_executable("qemu-system-i386")
    qemu64 = spawn.find_executable("qemu-system-x86_64")
    #   Cross compiler directory (required to build)
    cross_dir = os.path.expanduser("~/cross/bin/")
#=========================================================================
#End configuration section.  You shouldn't need to change anything after this.

_scripts_dir = os.path.dirname(__file__)

bochs_config = os.path.join(_scripts_dir,"_config-bochs.txt")
bochsdbg_config = os.path.join(_scripts_dir,"_config-bochsdbg.txt")
bochs_log = os.path.join(_scripts_dir,"../_bochs-log.txt")

vmdk = os.path.join(_scripts_dir,"../../../VMs/MOSS-disk-flat.vmdk")
