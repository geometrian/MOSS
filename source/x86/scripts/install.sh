SCRIPTDIR=$(dirname "$0")

echo "NOT installing to USB!  If you want to do that, run \"$SCRIPTDIR/_install_usb.sh\" specifically after carefully making sure it has the correct device."
#$SCRIPTDIR/_install_vhd.sh
$SCRIPTDIR/_install_vmdk.sh
#$SCRIPTDIR/_install_usb.sh
