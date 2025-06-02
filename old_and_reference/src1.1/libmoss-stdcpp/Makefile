$(shell echo 1>&2 "\e[32mBuild for libmoss-stdcpp\e[0m")

ifdef BUILD_DIR
	BUILD_DIR := $(BUILD_DIR)
	BUILD_SUBDIR := ../$(BUILD_DIR)/libmoss-stdcpp
else
	$(error BUILD_DIR must be set)
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

ifdef FLAGS_AR
	FLAGS_AR := $(FLAGS_AR)
else
	$(error FLAGS_AR must be set)
endif

$(info Make CC=$(CC))
$(info Make AR=$(AR))



#FLAGS_CC += -I../libmoss-stdc/

OBJS_CPP := $(patsubst %.cpp,$(BUILD_SUBDIR)/%.o,$(wildcard *.cpp))
OBJS := $(OBJS_CPP)

DEPS_CPP := $(patsubst $(BUILD_SUBDIR)/%.o,$(BUILD_SUBDIR)/%.d,$(OBJS_CPP))
DEPS := $(DEPS_CPP)

OUT_PATH := $(BUILD_SUBDIR)/libmoss-stdcpp.a



default: build

#build: builddir
#
#builddir:
#	mkdir -p $(BUILD_SUBDIR)



#Compiles ".cpp" files to ".o" files, also generating ".d" ("-MD -MP") dependencies.
$(BUILD_SUBDIR)/%.o: %.cpp
	$(CC)   $(FLAGS_CC) $(FLAGS_CPP)   -o $@   -c $<
#Bring in the more-specific rules of the dependencies (not available on first compile; ignore error
#	with "-").
-include $(DEPS)

$(OUT_PATH): $(OBJS)
	$(AR)   $(FLAGS_AR)   $(OUT_PATH) $(OBJS)

build: builddir $(OUT_PATH)

builddir:
	mkdir -p $(BUILD_SUBDIR)

clean:
	rm   -f   $(OBJS) $(DEPS)   $(OUT_PATH)
	if [ -d $(BUILD_SUBDIR) ]; then rmdir $(BUILD_SUBDIR); fi

print:
	$(info Build to $(BUILD_SUBDIR)/)
	$(info   Objects: $(OBJS))
	$(info   Deps:    $(DEPS))
	$(info   Output:  $(OUT_PATH))
