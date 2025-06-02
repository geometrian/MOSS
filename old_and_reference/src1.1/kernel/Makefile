$(shell echo 1>&2 "\e[32mBuild for kernel\e[0m")

ifdef BUILD_DIR
	BUILD_DIR := $(BUILD_DIR)
	BUILD_SUBDIR := ../$(BUILD_DIR)/kernel
else
	$(error BUILD_DIR must be set)
endif

ifdef FLAGS_ASM
	FLAGS_ASM := $(FLAGS_ASM)
else
	$(error FLAGS_ASM must be set)
endif

ifdef FLAGS_CC
	FLAGS_CC := $(FLAGS_CC)
else
	$(error FLAGS_CC must be set)
endif
ifdef FLAGS_CPP
	FLAGS_CPP := $(FLAGS_CPP)
else
	$(error FLAGS_CPP must be set)
endif

ifdef FLAGS_L
	FLAGS_L := $(FLAGS_L)
endif



#FLAGS_CC += -ffreestanding
#FLAGS_CC += -I../libmoss-stdc/
FLAGS_CC += -I$(UEFI_ROOT)MdePkg/Include/ -I$(UEFI_ROOT)MdePkg/Include/X64
#FLAGS_L += -nodefaultlib -filealign:16

FLAGS_L += -subsystem:efi_application -dll
FLAGS_L += -entry:efi_main

OBJS_ASM := $(patsubst %.asm,$(BUILD_SUBDIR)/%.o,$(wildcard *.asm))
OBJS_CPP := $(patsubst %.cpp,$(BUILD_SUBDIR)/%.o,$(wildcard *.cpp))
OBJS := $(OBJS_ASM) $(OBJS_CPP)

DEPS_ASM := $(patsubst $(BUILD_SUBDIR)/%.o,$(BUILD_SUBDIR)/%.o.d,$(OBJS_ASM))
DEPS_CPP := $(patsubst $(BUILD_SUBDIR)/%.o,$(BUILD_SUBDIR)/%.d,$(OBJS_CPP))
DEPS := $(DEPS_ASM) $(DEPS_CPP)

LIBS := ../$(BUILD_DIR)/libmoss-util/libmoss-util.a
LIBS += ../$(BUILD_DIR)/libmoss-stdcpp/libmoss-stdcpp.a
LIBS += ../$(BUILD_DIR)/libmoss-std-freestanding/libmoss-stdc.a

OUT_PATH1 := $(BUILD_SUBDIR)/BOOTX64.EFI
OUT_PATH2 := $(BUILD_SUBDIR)/BOOTX64.lib



default: build



##Compiles ".asm"/".cpp" files to ".o" files, also generating ".d" ("-MD -MP") dependencies.
#$(BUILD_SUBDIR)/%.o: %.asm
#	$(ASM)   $(FLAGS_ASM)               -o $@      $<
#$(BUILD_SUBDIR)/%.o: %.cpp
#	$(CC)    $(FLAGS_CC) $(FLAGS_CPP)   -o $@   -c $<
##Bring in the more-specific rules of the dependencies (not available on first compile; ignore error
##	with "-").
#-include $(DEPS)

#$(OUT_PATH1): $(LIBS) $(OBJS)
#	$(LLD)   $(FLAGS_L)   -out:$(OUT_PATH1)   $(OBJS) $(LIBS)



FLAGS_CC := -I$(UEFI_ROOT)MdePkg/Include/ -I$(UEFI_ROOT)MdePkg/Include/X64
FLAGS_CC += -ffreestanding -fshort-wchar -target x86_64-pc-win32-coff

FLAGS_CPP := -std=c++20

FLAGS_L := -subsystem:efi_application -dll
FLAGS_L += -entry:efi_main
FLAGS_L += -nodefaultlib -filealign:16

$(BUILD_SUBDIR)/main.o: main.cpp
	$(CC)    $(FLAGS_CC) $(FLAGS_CPP)   -o $@   -c $<
$(OUT_PATH1): $(BUILD_SUBDIR)/main.o
	$(LLD)   $(FLAGS_L)   -out:$(OUT_PATH1)   $(BUILD_SUBDIR)/main.o



build: builddir $(OUT_PATH1)

builddir:
	mkdir -p $(BUILD_SUBDIR)

clean:
	rm   -f   $(OBJS) $(DEPS)   $(OUT_PATH1) $(OUT_PATH2)
	if [ -d $(BUILD_SUBDIR) ]; then rmdir $(BUILD_SUBDIR); fi

print:
	$(info Build to $(BUILD_SUBDIR)/)
	$(info   Objects: $(OBJS))
	$(info   Deps:    $(DEPS))
	$(info   Output:  $(OUT_PATH1) $(OUT_PATH2))
