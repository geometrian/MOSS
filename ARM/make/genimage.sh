#!/bin/sh

OUTPUTNAME=moss-arm.img
OUTPUTDIR=../
OUTPUTSIZEmb=34
ROOTDIR=../root/


OUTPUTSIZEb=$(( $OUTPUTSIZEmb * 1024 * 1024 ))
OUTPUTCYLINDERS=$(( $OUTPUTSIZEb / 255 / 63 / 512 ))
OUTPUTBLOCKCOUNT=$(( $OUTPUTSIZEb / 512 ))


dd bs=512 count=$OUTPUTBLOCKCOUNT if=/dev/zero of=$OUTPUTDIR$OUTPUTNAME


fdisk -b 512 -H 255 -S 63 -C $OUTPUTCYLINDERS $OUTPUTDIR$OUTPUTNAME > /dev/null 2>&1 << EOF


losetup -o $((63*512)) /dev/loop1 $OUTPUTDIR$OUTPUTNAME
mkfs.msdos -F 32 /dev/loop1
losetup -d /dev/loop1

#mount -o loop,offset=$((63*512)) $OUTPUTDIR$OUTPUTNAME /media/sdcard

##chmod 777 $ROOTDIR/*
#cp $ROOTDIR/* /media/sdcard
#umount /media/sdcard