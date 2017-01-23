#http://stackoverflow.com/questions/454899/how-to-convert-flat-raw-disk-image-to-vmdk-for-virtualbox-or-vmplayer

SCRIPTDIR=$(dirname "$0")
SOURCEDIR=$SCRIPTDIR/..

#Works for VirtualBox, QEMU, and VMware
qemu-img convert -O vmdk $SOURCEDIR/.build/moss-disk.bin $SOURCEDIR/../../VMs/MOSS-disk-flat.vmdk
