#!/bin/sh
#COMPILER=arm-none-eabi
COMPILER=~/devel/rpi/tools/arm-bcm2708/arm-bcm2708hardfp-linux-gnueabi/bin/arm-bcm2708hardfp-linux-gnueabi
code=../source
boot=../root/boot/
 
# First let's compile our assembly file.
$COMPILER-as $code/init.asm -o $code/init.o
 
# Then let's call the linker to generate a binary output.
$COMPILER-ld $code/init.o -o $code/kernel.elf
 
# Then let's dump some debug info to disk.
#$COMPILER-objdump -D $code/init.o > $code/init.disassembled.asm
#$COMPILER-objdump -D $code/kernel.elf > $code/kernel.disassembled.asm
 
# Lastly let's copy our binary into an img format.
$COMPILER-objcopy $code/kernel.elf -O binary $boot/kernel.img
 
echo "Finished compiling!"