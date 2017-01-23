#Install to USB
#Will take a while: ~130 seconds for a 52MB image

SCRIPTDIR=$(dirname "$0")
SOURCEDIR=$SCRIPTDIR/..

sudo dd if=$SOURCEDIR/.build/moss-disk.bin of=/dev/sdb
