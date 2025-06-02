#http://docs.openstack.org/image-guide/content/ch_converting.html

SCRIPTDIR=$(dirname "$0")
SOURCEDIR=$SCRIPTDIR/..

#Works for VirtualPC
qemu-img convert -O vpc $SOURCEDIR/.build/moss-disk.bin $SOURCEDIR/../../VMs/MOSS-disk-flat.vhd
