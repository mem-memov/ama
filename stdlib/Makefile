.RECIPEPREFIX := >
.DEFAULT_GOAL := all

ROOT := $(abspath ..)
C_COMPILER ?= gcc
C_COMPILER_FLAGS ?= -O2 -Wall -Wextra -std=c11 -pedantic -D_POSIX_C_SOURCE=200809L
C_ARCHIVER ?= ar
BUILD_DIRECTORY := $(ROOT)/build/stdlib
INCLUDE_DIRECTORY := $(BUILD_DIRECTORY)/include
ARCHIVE := $(BUILD_DIRECTORY)/libama_std.a
MODULE_DIRECTORIES := modules/sys/math modules/sys/io
MODULE_FILES := $(foreach directory,$(MODULE_DIRECTORIES),$(wildcard $(directory)/*))
C_SOURCES := $(sort $(filter %.c,$(MODULE_FILES)))
INTERFACES := $(filter-out %.c %.h,$(MODULE_FILES))
OBJECTS := $(patsubst modules/%.c,$(BUILD_DIRECTORY)/%.o,$(C_SOURCES))
HEADER_STAMP := $(INCLUDE_DIRECTORY)/.headers-ready

# Extensionless Ama interfaces are source data, never implicit C executable targets.
$(INTERFACES): ;

.DELETE_ON_ERROR:
.PHONY: all clean
all: $(ARCHIVE)

$(ROOT)/ama:
>    $(MAKE) -C $(ROOT) ama C_COMPILER="$(C_COMPILER)" C_COMPILER_FLAGS="$(C_COMPILER_FLAGS)"

$(HEADER_STAMP): $(INTERFACES) $(ROOT)/ama Makefile
>    AMA_STDLIB= $(ROOT)/ama --generate-c-headers modules $(INCLUDE_DIRECTORY)
>    @touch $@

$(BUILD_DIRECTORY)/%.o: modules/%.c $(HEADER_STAMP)
>    @mkdir -p $(dir $@)
>    $(C_COMPILER) $(C_COMPILER_FLAGS) -I$(INCLUDE_DIRECTORY) -MMD -MP -c $< -o $@

$(ARCHIVE): $(OBJECTS) Makefile
>    $(RM) $@
>    $(C_ARCHIVER) rcs $@ $(OBJECTS)

-include $(OBJECTS:.o=.d)

clean:
>    $(RM) -r $(BUILD_DIRECTORY)
