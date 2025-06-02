BUILD_DIR := ../.build

BOOTBOOT_ROOT := /mnt/d/projects/programming/3rd-party-repos/bootboot
UEFI_ROOT     := /mnt/d/projects/programming/3rd-party-repos/edk2

#Disk image parameters
#	To change, see "doc/building.md".
#		Size of disk (in MiB)
DISK_IMG_SZ      := 64
#		Offset (in sectors) and size (in MiB) of first partition
#			Note: FAT requires the size to be at least (sector size) ⨯ 65527 ≈ 32 MiB.
DISK_IMG_PT1_OFF := 2048
DISK_IMG_PT1_SZ  := 32

CFG_DEBREL := MOSS_DEBUG
#CFG_DEBREL := MOSS_RELEASE
#CFG_ARCH := MOSS_ARCH_IA32
CFG_ARCH := MOSS_ARCH_AMD64

CC  := clang
ASM := nasm
#LLD := lld-link
LLD := ld.lld
#-target elf64-x86-64
AR  := llvm-ar



#TARGET := -target x86_64-pc-win32-coff

FLAGS_ASM := -MP   -f elf64

FLAGS_C := -MD -MP   --target=x86_64-unknown-none-elf -masm=intel
#-mfma
#FLAGS_C += -mno-sse
FLAGS_C += -mno-avx
FLAGS_C += -Wall -Werror -Wswitch -Wcovered-switch-default
FLAGS_C += -Wno-constant-logical-operand -Wno-unused-function -Wno-frame-address
FLAGS_C += -Wno-error=\#warnings -Wno-error=unused-variable -Wno-error=unused-label
FLAGS_C += -std=c2x
ifeq       ($(CFG_DEBREL),MOSS_DEBUG)
	FLAGS_C += -g -fno-omit-frame-pointer
	FLAGS_C += -fstack-protector-all
	#FLAGS_C += -fsanitize=address -fsanitize-address-use-after-return=always
	#FLAGS_C += -fsanitize=undefined
else ifeq  ($(CFG_DEBREL),MOSS_RELEASE)
	FLAGS_C += -DNDEBUG -O3
	FLAGS_C += -fno-stack-protector
else
	$(error must define build config `MOSS_DEBUG` or `MOSS_RELEASE`!)
endif

#FLAGS_CPP := -std=c++23

FLAGS_AR := -rsc

#The red zone is not interrupt-safe.  See: https://os.phil-opp.com/red-zone/
FLAGS_KC := $(FLAGS_C) -mno-red-zone -fpic
FLAGS_KC += -ffreestanding -nostdlib
#FLAGS_KC += -ffreestanding -nostdlib -nostdinc
#-ibuiltininc
FLAGS_KC += -DMOSS_ENV=1 -isystemlibmoss-stdc/

#FLAGS_KCPP := $(FLAGS_CPP) -fno-rtti -fno-exceptions

#FLAGS_KL := -nodefaultlib
#-subsystem:console
FLAGS_KL := -T kernel/bootboot.ld   -Map $(BUILD_DIR)/kernel/moss.map
ifeq       ($(CFG_DEBREL),MOSS_DEBUG)
else ifeq  ($(CFG_DEBREL),MOSS_RELEASE)
	FLAGS_KL += --strip-all
endif



ASMS_LIBC := $(shell find libmoss-stdc/ -name *.asm)
SRCS_LIBC := $(shell find libmoss-stdc/ -name *.c)

OBJS_LIBCi := $(patsubst libmoss-stdc/%.asm,%.o,$(ASMS_LIBC))
OBJS_LIBCi += $(patsubst libmoss-stdc/%.c,%.o,$(SRCS_LIBC))

OBJS_LIBC0 := $(patsubst %.o,$(BUILD_DIR)/libmoss-stdc-userland/%.o,$(OBJS_LIBCi))
OBJS_LIBC1 := $(patsubst %.o,$(BUILD_DIR)/libmoss-stdc-freestanding/%.o,$(OBJS_LIBCi))

DEPS_LIBC := $(patsubst %.o,%.d,$(OBJS_LIBC0))
DEPS_LIBC += $(patsubst %.o,%.d,$(OBJS_LIBC1))



SRCS_LIBU := $(wildcard libmoss-util/*.c)

OBJS_LIBU := $(patsubst libmoss-util/%.c,$(BUILD_DIR)/libmoss-util-freestanding/%.o,$(SRCS_LIBU))

DEPS_LIBU := $(patsubst %.o,$(BUILD_DIR)/%.d,$(OBJS_LIBU))



ASMS_K := $(shell find kernel/ -name *.asm)
SRCS_K := $(shell find kernel/ -name *.c)

OBJS_K := $(patsubst %.asm,$(BUILD_DIR)/%.o,$(ASMS_K))
OBJS_K += $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRCS_K))

DEPS_K := $(patsubst %.o,$(BUILD_DIR)/%.d,$(OBJS_K))



DEPS := $(DEPS_LIBC) $(DEPS_LIBU) $(DEPS_K)



#$(info   Objects: $(OBJS_LIBCi))
#$(info   Objects: $(OBJS_LIBC0))
#$(info   Deps:    $(DEPS))



default: kernel



#Compiles ".asm"/".c"/".cpp" files to ".o" files, also generating ".d" ("-MD -MP") dependencies.
#	See also https://www.gnu.org/software/make/manual/html_node/Automatic-Variables.html

$(BUILD_DIR)/kernel/%.o:   kernel/%.c
	$(CC)    $(FLAGS_KC)   -I$(BOOTBOOT_ROOT)/dist/   -o $@   -c $<
$(BUILD_DIR)/kernel/%.o:   kernel/%.asm
	$(ASM)   $(FLAGS_ASM)   -MD $(BUILD_DIR)/kernel/$*.d    -o $@   $<

$(BUILD_DIR)/libmoss-stdc-freestanding/%.o:   libmoss-stdc/%.asm
	$(ASM)   $(FLAGS_ASM)   -MD $(BUILD_DIR)/libmoss-stdc-freestanding/$*.d    -o $@   $<
$(BUILD_DIR)/libmoss-stdc-freestanding/%.o:   libmoss-stdc/%.c
	$(CC)    $(FLAGS_KC)   -DMOSS_ENV=1   -o $@   -c $<

$(BUILD_DIR)/libmoss-util-freestanding/%.o:   libmoss-util/%.c
	$(CC)    $(FLAGS_KC)   -o $@   -c $<

#$(BUILD_DIR)/%.o: %.asm
#	$(ASM)   $(FLAGS_ASM)   -o $@      $<
#$(BUILD_DIR)/%.o: %.c
#	$(CC)    $(FLAGS_C)    -I$(BOOTBOOT_ROOT)/dist/   -o $@   -c $<

#Bring in the more-specific rules of the dependencies (not available on first compile; ignore error
#	with "-").
-include $(DEPS)



_kernel_begin:
	$(shell echo 1>&2 "\e[32mBuilding kernel . . .\e[0m")
	#mkdir -p $(BUILD_DIR)/kernel/
	mkdir -p $(BUILD_DIR)/kernel/x86
	mkdir -p $(BUILD_DIR)/kernel/device

_kernel_boot_bootboot:   _kernel_fonts   $(OBJS_K)
	$(LLD)   $(FLAGS_KL)   -o $(BUILD_DIR)/kernel/moss.x86_64.elf \
		$(OBJS_K)   $(BUILD_DIR)/kernel/font.o \
		$(BUILD_DIR)/libmoss-util-freestanding/libmoss-util.a\
		$(BUILD_DIR)/libmoss-stdc-freestanding/libmoss-stdc.a

_kernel_boot_bios:

_kernel_boot_limine:

_kernel_boot_uefi:

_kernel_fonts:   data/fonts/Tamsyn8x16b.psf
	objcopy   -I binary data/fonts/Tamsyn8x16b.psf   -O elf64-x86-64 $(BUILD_DIR)/kernel/font.o
	#objcopy   -I binary data/fonts/font5_5x9.psf   -O elf64-x86-64 $(BUILD_DIR)/kernel/font.o

kernel:   libmoss-stdc libmoss-util   _kernel_begin _kernel_boot_bootboot
	$(shell echo 1>&2 "\e[32mBuilt MOSS!  (\e[33mNote not deployed yet!\e[32m)\e[0m")

_libmoss-stdc_begin:
	$(shell echo 1>&2 "\e[32mBuilding libmoss-stdc . . .\e[0m")
	mkdir -p $(BUILD_DIR)/libmoss-stdc-freestanding/
_libmoss-stdc-userland:   $(OBJS_LIBC0)
	$(AR)   $(FLAGS_AR)   $(BUILD_DIR)/libmoss-stdc-userland/libmoss-stdc.a   $(OBJS_LIBC0)
_libmoss-stdc-freestanding:   $(OBJS_LIBC1)
	$(AR)   $(FLAGS_AR)   $(BUILD_DIR)/libmoss-stdc-freestanding/libmoss-stdc.a   $(OBJS_LIBC1)
libmoss-stdc:   _libmoss-stdc_begin   _libmoss-stdc-freestanding

_libmoss-util_begin:
	$(shell echo 1>&2 "\e[32mBuilding libmoss-util . . .\e[0m")
	mkdir -p $(BUILD_DIR)/libmoss-util-freestanding/
_libmoss-util-freestanding:   $(OBJS_LIBU)
	$(AR)   $(FLAGS_AR)   $(BUILD_DIR)/libmoss-util-freestanding/libmoss-util.a   $(OBJS_LIBU)
libmoss-util:   _libmoss-util_begin   _libmoss-util-freestanding




disk_create: $(BUILD_DIR)/diskimage.img
	dd   if=/dev/zero of=$(BUILD_DIR)/diskimage.img   bs=1048576 count=$(DISK_IMG_SZ)
	sfdisk   $(BUILD_DIR)/diskimage.img < diskimage$(DISK_IMG_SZ)M.cfg
	sudo losetup   -o $$(( $(DISK_IMG_PT1_OFF)*512 ))   --sizelimit $$(( $(DISK_IMG_PT1_SZ)*1024*1024 ))   -f $(BUILD_DIR)/diskimage.img
	losetup -a
	sudo mkfs.vfat   -F 16   -n "EFI System"   /dev/loop0
	sudo losetup -d /dev/loop0

.PHONY: disk_mount disk_unmount
disk_mount:
	sudo losetup   -o $$(( $(DISK_IMG_PT1_OFF)*512 ))   --sizelimit $$(( $(DISK_IMG_PT1_SZ)*1024*1024 ))   -f $(BUILD_DIR)/diskimage.img
	losetup -a
	mkdir $(BUILD_DIR)/mounted_fs/
	sudo mount   /dev/loop0   $(BUILD_DIR)/mounted_fs/   -o umask=0022,gid=33,uid=33
disk_unmount:
	sudo umount $(BUILD_DIR)/mounted_fs/
	rmdir $(BUILD_DIR)/mounted_fs/
	sudo losetup   -d /dev/loop0

.PHONY: disk_deploy_bootboot
disk_deploy_bootboot:   kernel
	sudo mkdir   -p   $(BUILD_DIR)/mounted_fs/EFI/BOOT/
	sudo mkdir   -p   $(BUILD_DIR)/mounted_fs/BOOTBOOT/
	sudo cp   kernel/bootboot.efi $(BUILD_DIR)/mounted_fs/EFI/BOOT/BOOTX64.EFI
	sudo cp   kernel/bootboot.cfg $(BUILD_DIR)/mounted_fs/BOOTBOOT/CONFIG
	sudo cp   -r   root_fs/* $(BUILD_DIR)/mounted_fs/
	sudo mkdir   -p   $(BUILD_DIR)/initrd/sys/
	sudo cp   $(BUILD_DIR)/kernel/moss.x86_64.elf $(BUILD_DIR)/initrd/sys/moss.x86_64.elf
	#sudo cp   $(BUILD_DIR)/kernel/moss.map $(BUILD_DIR)/initrd/sys/moss.map
	sudo tar   -czf $(BUILD_DIR)/mounted_fs/BOOTBOOT/INITRD $(BUILD_DIR)/initrd/*

#VMDK works for VirtualBox, QEMU, and VMware
#	Note VB wants a consistent UUID, so we have to set it explicitly.
disk_deploy_vb:
	qemu-img   convert   -O vmdk   $(BUILD_DIR)/diskimage.img   $(BUILD_DIR)/diskimage.vmdk
	VBoxManage   internalcommands   sethduuid   $(BUILD_DIR)/diskimage.vmdk   3e35d8c1-f057-4b4c-9d9e-120881738019

deploy:   kernel   disk_mount disk_deploy_bootboot disk_unmount   disk_deploy_vb
	$(shell echo 1>&2 "\e[92m#### Built and deployed MOSS! ####\e[0m")





makedisk: $(BUILD_DIR)/diskimage.img













#$(BUILD_DIR)/$(SUBDIR)/%.o: $(SUBDIR)/%.asm
#	$(ASM)   $(FLAGS_ASM)               -o $@      $<
#
#$(BUILD_DIR)/kernel/$(SUBDIR)/%.o: kernel/$(SUBDIR)/%.c
#	$(CC)    $(FLAGS_C)   -o $@   -c $<
#
#$(BUILD_DIR)/kernel/$(SUBDIR)/%.o: kernel/$(SUBDIR)/%.cpp
#	$(CC)    $(FLAGS_CC) $(FLAGS_CPP)   -o $@   -c $<
#
#$(BUILD_DIR)/moss.x86_64.elf: $(BUILD_SUBDIR)/main.o
#	$(LLD)   $(FLAGS_L)   -out:$(BUILD_DIR)/moss.x86_64.elf   $(BUILD_SUBDIR)/main.o



#$(BUILD_DIR)/diskimage.img: makedirs
#	dd   if=/dev/zero of=$(BUILD_DIR)/diskimage.img   bs=1048576 count=$(DISK_IMG_SZ)
#	sgdisk   $(BUILD_DIR)/diskimage.img   --new=1:2048 --typecode 1:ef00
#	mformat   -i $(BUILD_DIR)/diskimage.img@@1M
#	mmd       -i $(BUILD_DIR)/diskimage.img@@1M ::/EFI ::/EFI/BOOT ::/BOOTBOOT ::/sys
#	mcopy     -i $(BUILD_DIR)/diskimage.img@@1M kernel/bootboot.efi ::/EFI/BOOT
#	mcopy     -i $(BUILD_DIR)/diskimage.img@@1M kernel/bootboot.cfg ::/BOOTBOOT/CONFIG
#	mcopy     -i $(BUILD_DIR)/diskimage.img@@1M $(BUILD_DIR)/kernel/moss.x86_64.elf ::/sys








#	mkdir -r tmp/sys
#	cp mykernel.x86_64.elf tmp/sys/core
#	# copy more files to tmp/ directory
#
#	# create your file system image or an archive. For example use one of these:
#	cd tmp
#	find . | cpio -H newc -o | gzip > ../INITRD
#	find . | cpio -H crc -o | gzip > ../INITRD
#	find . | cpio -H hpodc -o | gzip > ../INITRD
#	tar -czf ../INITRD *
#	mkfs ../INITRD .
#
#
#CFLAGS = -Wall -fpic -ffreestanding -fno-stack-protector -nostdinc -nostdlib -I../../dist/
#LDFLAGS =  -nostdlib -n -T link.ld
#STRIPFLAGS =  -s -K mmio -K fb -K bootboot -K environment -K initstack
#
#all: mykernel.x86_64.elf mykernel.aarch64.elf mykernel.riscv64.elf
#
#mykernel.x86_64.elf: kernel.c
#	x86_64-elf-gcc $(CFLAGS) -mno-red-zone -c kernel.c -o kernel.o
#	x86_64-elf-ld -r -b binary -o font.o font.psf
#	x86_64-elf-ld $(LDFLAGS) kernel.o font.o -o mykernel.x86_64.elf
#	x86_64-elf-strip $(STRIPFLAGS) mykernel.x86_64.elf
#	x86_64-elf-readelf -hls mykernel.x86_64.elf >mykernel.x86_64.txt
#
#mykernel.aarch64.elf: kernel.c
#	aarch64-elf-gcc $(CFLAGS) -c kernel.c -o kernel.o
#	aarch64-elf-ld -r -b binary -o font.o font.psf
#	aarch64-elf-ld $(LDFLAGS) kernel.o font.o -o mykernel.aarch64.elf
#	aarch64-elf-strip $(STRIPFLAGS) mykernel.aarch64.elf
#	aarch64-elf-readelf -hls mykernel.aarch64.elf >mykernel.aarch64.txt
#
#mykernel.riscv64.elf: kernel.c
#	riscv64-elf-gcc $(CFLAGS) -c kernel.c -o kernel.o
#	riscv64-elf-ld -r -b binary -o font.o font.psf
#	riscv64-elf-ld $(LDFLAGS) kernel.o font.o -o mykernel.riscv64.elf
#	riscv64-elf-strip $(STRIPFLAGS) mykernel.riscv64.elf
#	riscv64-elf-readelf -hls mykernel.riscv64.elf >mykernel.riscv64.txt
#
#clean:
#	rm *.o *.elf *.txt || true

