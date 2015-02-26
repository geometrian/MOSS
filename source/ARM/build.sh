name=moss-arm.img
size_mb=34


size_b=$(( $size_mb * 1024 * 1024 ))
cyls=$(( $size_b / 255 / 63 / 512 ))
blocks=$(( $size_b / 512 ))

echo "Cylinders: " $cyls
echo "Blocks:    " $blocks


dd bs=512 count=$blocks if=/dev/zero of=$name


losetup /dev/loop0 $name

mkfs.vfat -F 32 /dev/loop0

mount -t vfat /dev/loop0 /mnt
cp root /mnt
umount /mnt

losetup -d /dev/loop

#fdisk -b 512 -H 255 -S 63 -C $cyls $name > /dev/null 2>&1 << EOF

#mkfs.vfat $name -n MOSS-ARM-FS

#mkisofs -V moss-arm.iso -r root > moss-arm.iso

#mkfs.vfat -F 32 /dev/hdb5
