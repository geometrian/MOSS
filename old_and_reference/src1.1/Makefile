BUILD_DIR := ../.build

CFG_DEBREL := MOSS_DEBUG
#CFG_DEBREL := MOSS_RELEASE
#CFG_ARCH := MOSS_ARCH_IA32
CFG_ARCH := MOSS_ARCH_AMD64
#CFG_OPTABM := -DMOSS_WITH_ABM #https://en.wikipedia.org/wiki/X86_Bit_manipulation_instruction_set#ABM_(Advanced_Bit_Manipulation)

UEFI_ROOT := /mnt/d/projects/programming/3rd-party-repos/edk2/

CC  := clang
ASM := nasm
LLD := lld-link
AR  := llvm-ar

#Disk image parameters
#	To change, see "doc/building.md".
#		Size of disk (in MiB)
DISK_IMG_SZ      := 64
#		Offset (in sectors) and size (in MiB) of first partition
#			Note: FAT requires the size to be at least (sector size) ⨯ 65527 ≈ 32 MiB.
DISK_IMG_PT1_OFF := 2048
DISK_IMG_PT1_SZ  := 32



FLAGS_ASM := -MD -MP
ifeq ($(CFG_ARCH),MOSS_ARCH_IA32)
	FLAGS_ASM += -f win32
	TARGET := -target x86-pc-win32-coff
endif
ifeq ($(CFG_ARCH),MOSS_ARCH_AMD64)
	FLAGS_ASM += -f win64
	TARGET := -target x86_64-pc-win32-coff
endif
#TARGET := -target x86_64-pc-none-gnu -I/usr/include/c++/13/ -I/usr/include/x86_64-linux-gnu/c++/13/

FLAGS_CC := -MD -MP
FLAGS_CC += $(TARGET)
FLAGS_CC += -masm=intel
FLAGS_CC += -Werror -Wno-error=\#warnings
FLAGS_CC += -fshort-wchar -fno-stack-protector -mno-red-zone
FLAGS_CC += -fno-rtti -fno-exceptions
FLAGS_CC += -Wswitch -Wcovered-switch-default
FLAGS_CC += -D$(CFG_ARCH) -D$(CFG_DEBREL)
ifeq       ($(CFG_DEBREL),MOSS_DEBUG)
	FLAGS_CC += -g
else ifeq  ($(CFG_DEBREL),MOSS_RELEASE)
	FLAGS_CC += -DNDEBUG -O3
endif
FLAGS_CC += $(CFG_OPTABM)

FLAGS_CPP := -std=c++2b

FLAGS_AR := -rsc



#https://gist.github.com/HiImJulien/3eb47d7d874fe5483810bd77940e74c0

#C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.38.33130\lib
MSVC_BASE    := /mnt/c/Program\ Files/Microsoft\ Visual\ Studio/2022/Community/VC/Tools/MSVC/14.38.33130
MSVC_INCLUDE := $(MSVC_BASE)/include
MSVC_LIB     := $(MSVC_BASE)/lib

WINSDK_BASE    := /mnt/c/Program\ Files\ \(x86\)/Windows\ Kits/10
WINSDK_VER     := 10.0.22621.0
WINSDK_INCLUDE := $(WINSDK_BASE)/Include/$(WINSDK_VER)
WINSDK_LIB     := $(WINSDK_BASE)/lib/$(WINSDK_VER)

FLAGS_CC_WIN := -D_CRT_SECURE_NO_WARNINGS -D_AMD64_
FLAGS_CC_WIN += -isystem$(MSVC_INCLUDE)
FLAGS_CC_WIN += -isystem$(WINSDK_INCLUDE)/ucrt
FLAGS_CC_WIN += -isystem$(WINSDK_INCLUDE)/shared
FLAGS_CC_WIN += -isystem$(WINSDK_INCLUDE)/um
FLAGS_CC_WIN += -isystem$(WINSDK_INCLUDE)/winrt

#FLAGS_L_WIN := /manifest:no
FLAGS_L_WIN := -L${MSVC_LIB}/x64
FLAGS_L_WIN += -L${WINSDK_LIB}/ucrt/x64
FLAGS_L_WIN += -L${WINSDK_LIB}/um/x64
#FLAGS_L_WIN += -l:libcmt.lib



export

.PHONY: kernel libmoss-stdc-userland libmoss-stdc-freestanding libmoss-stdc-embedwin libmoss-stdcpp libmoss-util clean print

default: build



kernel: libmoss-stdc-freestanding libmoss-util
	$(MAKE)   -C ./kernel/
libmoss-stdc-userland:
	$(MAKE)   -C ./libmoss-stdc/ MOSSC_CONFIG=userland
libmoss-stdc-freestanding:
	$(MAKE)   -C ./libmoss-stdc/ MOSSC_CONFIG=freestanding
libmoss-stdc-embedwin:
	$(MAKE)   -C ./libmoss-stdc/ MOSSC_CONFIG=embedwin
libmoss-stdcpp: libmoss-stdc-freestanding
	$(MAKE)   -C ./libmoss-stdcpp/
libmoss-util: libmoss-stdcpp
	$(MAKE)   -C ./libmoss-util/
tests: libmoss-stdc-embedwin
	$(MAKE)   -C ./tests/



all: makedisk makefs build tests copyfs copyvb

build: kernel
	$(shell echo 1>&2 "\e[32mBuilt MOSS!  (Note not deployed yet!)\e[0m")

deploy: build copyfs copyvb
	$(shell echo 1>&2 "\e[32mBuilt and deployed MOSS!\e[0m")

clean:
	$(MAKE)   -C ./kernel/            clean
	$(MAKE)   -C ./libmoss-stdc/      clean   MOSSC_CONFIG=userland
	$(MAKE)   -C ./libmoss-stdc/      clean   MOSSC_CONFIG=freestanding
	$(MAKE)   -C ./libmoss-stdc/      clean   MOSSC_CONFIG=embedwin
	$(MAKE)   -C ./libmoss-stdcpp/    clean
	$(MAKE)   -C ./libmoss-util/      clean

print:
	$(MAKE)   -C ./kernel/            print
	$(MAKE)   -C ./libmoss-stdc/      print   MOSSC_CONFIG=userland
	$(MAKE)   -C ./libmoss-stdcpp/    print
	$(MAKE)   -C ./libmoss-util/      print



makedisk:
	dd   if=/dev/zero of=$(BUILD_DIR)/diskimage.img   bs=1048576 count=$(DISK_IMG_SZ)
	sfdisk   $(BUILD_DIR)/diskimage.img < diskimage$(DISK_IMG_SZ)M.cfg

makefs:
	sudo losetup   -o $$(( $(DISK_IMG_PT1_OFF)*512 ))   --sizelimit $$(( $(DISK_IMG_PT1_SZ)*1024*1024 ))   -f $(BUILD_DIR)/diskimage.img
	losetup -a
	sudo mkfs.vfat   -F 16   -n "EFI System"   /dev/loop0
	sudo losetup -d /dev/loop0

copyfs:
	sudo losetup   -o $$(( $(DISK_IMG_PT1_OFF)*512 ))   --sizelimit $$(( $(DISK_IMG_PT1_SZ)*1024*1024 ))   -f $(BUILD_DIR)/diskimage.img
	losetup -a
	mkdir $(BUILD_DIR)/mounted_fs/
	sudo mount   /dev/loop0   $(BUILD_DIR)/mounted_fs/
	sudo mkdir   -p   $(BUILD_DIR)/mounted_fs/EFI/BOOT/
	sudo cp   $(BUILD_DIR)/kernel/BOOTX64.EFI $(BUILD_DIR)/mounted_fs/EFI/BOOT/BOOTX64.EFI
	sudo cp   -r   root_fs/* $(BUILD_DIR)/mounted_fs/
	sudo umount $(BUILD_DIR)/mounted_fs/
	rmdir $(BUILD_DIR)/mounted_fs/
	sudo losetup   -d /dev/loop0

#VMDK works for VirtualBox, QEMU, and VMware
#	Note VB wants a consistent UUID, so we have to set it explicitly.
copyvb:
	qemu-img   convert   -O vmdk   $(BUILD_DIR)/diskimage.img   $(BUILD_DIR)/diskimage.vmdk
	VBoxManage   internalcommands   sethduuid   $(BUILD_DIR)/diskimage.vmdk   3e35d8c1-f057-4b4c-9d9e-120881738019
#VBoxManage   clonemedium   --format VDI   $(BUILD_DIR)/diskimage.img $(BUILD_DIR)/diskimage.vdi
