#Sets up a common disk image that is used for building the VM disk or
#	installing to USB

#See: http://wiki.osdev.org/Loopback_Device
#See: http://superuser.com/questions/130955/how-to-install-grub-into-an-img-file
#See: http://unix.stackexchange.com/questions/53890/partitioning-disk-image-file

#If you get a mounting error, check:
#	https://forums.virtualbox.org/viewtopic.php?f=3&p=283645
#	sudo ln -f -s /opt/VBoxGuestAdditions-4.3.18/lib/VBoxGuestAdditions /usr/lib/VBoxGuestAdditions
#	sudo ln -f -s /opt/VBoxGuestAdditions-4.3.18/lib/VBoxGuestAdditions/mount.vboxsf /sbin/mount.vboxsf
#	You will need to reboot.

#Note: apparently kpartx will silently fail to map any partitions on a shared folder in a VM.
#	Therefore, all operations are done in a temporary "~/moss_disk_temp" in case this is the case!


SCRIPTDIR=$(dirname "$0")
SOURCEDIR=$SCRIPTDIR/..
TEMPDISK=$(cd ~/; pwd)/moss_disk_temp

function wait_for_enter() {
	read -p "  <Press ENTER to continue . . .>"
}


echo "######## MOSS beginning setup for release . . ."

export       remake_disk=0
export remake_partitions=0
export         remake_fs=0
export       remake_grub=0
export fs_ext2_fat=1 #0 is Ext2, 1 is FAT 32

#Note: on changing these, change also the Bochs configuration files.
export CYLINDERS=100
export HEADS=16
export SECTORS_PER_TRACK=63
export SECTOR_SIZE=512

#Make HDD image
if [ $remake_disk -eq 1 ]
then
	echo "######## Making HDD file . . ."
	dd if=/dev/zero of=$TEMPDISK bs=$((HEADS*SECTORS_PER_TRACK*SECTOR_SIZE)) count=$CYLINDERS

	wait_for_enter
else
	echo "######## NOT making HDD file."
fi


#Create partition table
if [ $remake_disk -eq 1 ]
then
	echo "######## Making partition table . . ."
	sudo parted $TEMPDISK mktable msdos

	echo "######## Making partition thereon . . ."
	if [ $fs_ext2_fat -eq 0 ]
	then
		cat <<-EOF | sudo parted  $TEMPDISK  mkpart
		primary
		ext2
		1
		-0
		EOF
	else
		cat <<-EOF | sudo parted  $TEMPDISK  mkpart
		primary
		fat32
		1
		-0
		EOF
	fi

	wait_for_enter
else
	echo "######## NOT making a partition table."
fi


#Make the disk image and its partitions into devices
echo "######## Making disk image and its partitions into devices . . ."
sudo losetup /dev/loop0 $TEMPDISK
sudo kpartx  -v -a -s  /dev/loop0 #See note at top of file
#wait_for_enter


#Make a file system on first partition
if [ $remake_fs -eq 1 ]
then
	echo "######## Making a filesystem . . ."

	if [ $fs_ext2_fat -eq 0 ]
	then
		#"m" is reserved blocks percentage.
		sudo mkfs  -t ext2  -m 1  -v  /dev/mapper/loop0p1
	else
		sudo mkfs  -t vfat  -F 32  /dev/mapper/loop0p1
	fi
	#Note: may have to unmount loops, restart, and then remount loops here.  See http://forum.osdev.org/viewtopic.php?f=1&t=26907

	wait_for_enter
else
	echo "######## NOT making a filesystem."
fi


#This next section actually puts a file system and GrUB on the disk.  It took a lot of work to get to work
#	since GrUB apparently wasn't intended (or at least tested sufficiently) to do this.  A lot is based on
#	http://www.slideshare.net/sukhdotin/installing-grub-on-virtual-hard-disk-images-5094625, but I found
#	http://www.omninerd.com/articles/Installing_GRUB_on_a_Hard_Disk_Image_File, which should be just about
#	the same.
#Also, this section puts the kernel and all of the needed files on the disk.
echo "######## Filling disk . . ."

#Mount the first partition onto "/mnt"
#	If you get an error here, see comment at top of file.
if [ $fs_ext2_fat -eq 0 ]
then
	sudo mount -t ext2 /dev/mapper/loop0p1 /mnt
else
	sudo mount -t vfat /dev/mapper/loop0p1 /mnt
fi
#wait_for_enter

#Set up first partition's /boot directory
echo "######## Copying over GrUB files . . ."
#I tried.  I really did, to use grub-install.  It just kept failing:
#	http://forum.osdev.org/viewtopic.php?f=1&t=26907
#	http://ebroder.net/2009/08/04/installing-grub-onto-a-disk-image/
#	http://superuser.com/questions/619571/installing-grub-legacy-on-virtual-disk
#At the suggestion of http://www.slideshare.net/sukhdotin/installing-grub-on-virtual-hard-disk-images-5094625,
#	just copy the needed files over.
sudo mkdir -p /mnt/boot/grub
sudo cp $SOURCEDIR/source_moss/grub/stage1 /mnt/boot/grub/stage1
sudo cp $SOURCEDIR/source_moss/grub/stage2 /mnt/boot/grub/stage2
sudo cp $SOURCEDIR/source_moss/grub/e2fs_stage1_5 /mnt/boot/grub/e2fs_stage1_5
#wait_for_enter

#Copy the kernel over and the facility to boot it
echo "######## Copying over kernel files . . ."
sudo cp $SOURCEDIR/.build/MOSS.bin /mnt/boot/MOSS.bin
sudo cp $SOURCEDIR/source_moss/grub/menu.lst /mnt/boot/grub/menu.lst
echo "######## Copying over kernel data files . . ."
sudo cp -r $SOURCEDIR/filesystem/ /mnt/
#wait_for_enter

#Setup GrUB device map.  I've done this before, but never passing /dev/null in . . .
if [ $remake_grub -eq 1 ]
then
	echo "######## Setting up GrUB . . ."
	grub --device-map=/dev/null <<-EOF
	device (hd0) $TEMPDISK
	geometry (hd0) $((CYLINDERS)) $((HEADS)) $((SECTORS_PER_TRACK))
	root (hd0,0)
	setup (hd0)
	quit
	EOF
	wait_for_enter
else
	echo "######## NOT setting up GrUB."
fi

#Unmount the first partition from "/mnt"
sudo umount /dev/mapper/loop0p1
#wait_for_enter


#Unmake the disk image and its partitions into devices
echo "######## Unmaking disk image and its partitions as devices . . ."
sudo kpartx -d /dev/loop0
sudo losetup -d /dev/loop0
#wait_for_enter


#Copy final result
#	Note not moving so that can avoid e.g. remaking it if we rerun this script
cp $TEMPDISK $SOURCEDIR/.build/moss-disk.bin


#Done
echo "######## Preparation for release complete!"
