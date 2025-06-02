$(shell echo 1>&2 "\e[32mBuild for tests\e[0m")

ifdef BUILD_DIR
	BUILD_DIR := $(BUILD_DIR)
	BUILD_SUBDIR := ../$(BUILD_DIR)/tests
else
	$(error BUILD_DIR must be set)
endif

ifdef FLAGS_CC
	FLAGS_CC := $(FLAGS_CC)
else
	$(error FLAGS_CC must be set)
endif

#ifdef FLAGS_L
#	FLAGS_L := $(FLAGS_L)
#else
#	$(error FLAGS_L must be set)
#endif



FLAGS_CC += -I../libmoss-stdc/ -fuse-ld=lld
FLAGS_L += -subsystem:console /machine:x64

##FLAGS_L += -subsystem:efi_application -dll
##FLAGS_L += -entry:efi_main
#
#OBJS_CPP := $(patsubst %.cpp,$(BUILD_SUBDIR)/%.o,$(wildcard *.cpp))
#OBJS := $(OBJS_ASM) $(OBJS_CPP)
#OBJS := $(OBJS)
#
#DEPS_CPP := $(patsubst $(BUILD_SUBDIR)/%.o,$(BUILD_SUBDIR)/%.d,$(OBJS_CPP))
#DEPS := $(DEPS_ASM) $(DEPS_CPP)

LIBS := ../$(BUILD_DIR)/libmoss-stdc-embedwin/libmoss-stdc.a



default: build



#Compiles ".cpp" files to ".o" files, also generating ".d" ("-MD -MP") dependencies.
$(BUILD_SUBDIR)/%.o: %.cpp
	$(CC)    $(FLAGS_CC) $(FLAGS_CPP) $(FLAGS_CC_WIN)   -o $@   -c $<
#Bring in the more-specific rules of the dependencies (not available on first compile; ignore error
#	with "-").
-include $(DEPS)

$(BUILD_SUBDIR)/test_printf.exe: $(LIBS) $(BUILD_SUBDIR)/test_printf.o
	$(LLD)   $(FLAGS_L)   -out:$(BUILD_SUBDIR)/test_printf.exe   $(LIBS) $(BUILD_SUBDIR)/test_printf.o



build: builddir $(BUILD_SUBDIR)/test_printf.exe

builddir:
	mkdir -p $(BUILD_SUBDIR)

#clean:
#	rm   -f   $(OBJS) $(DEPS)   $(BUILD_SUBDIR)/BOOTX64.EFI $(BUILD_SUBDIR)/BOOTX64.lib
#	if [ -d $(BUILD_SUBDIR) ]; then rmdir $(BUILD_SUBDIR); fi

#print:
#	$(info Build to $(BUILD_SUBDIR)/)
#	$(info   Objects: $(OBJS))
#	$(info   Deps:    $(DEPS))
