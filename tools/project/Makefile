# @file
# @brief Build the independent project executable without linking compiler dispatch or state.
# Contract: ../../docs/implementation/project-tool.md; PROJECT-01 separates command ownership.
.RECIPEPREFIX := >
.DEFAULT_GOAL := all
.DELETE_ON_ERROR:
ROOT := $(abspath ../..)
C_COMPILER ?= gcc
C_COMPILER_FLAGS ?= -O2 -Wall -Wextra -std=c11 -pedantic -D_POSIX_C_SOURCE=200809L
include $(ROOT)/make/sources.mk
include sources.mk
PROJECT_TOOL_SHARED_SOURCES += $(PROJECT_TOOL_CAPTURE_SOURCES)
LOCAL_SOURCES := $(patsubst ./%,%,$(call source_files,.))
CONFIGURATION_DIRECTORY := $(ROOT)/build/project-tool
OBJECTS := $(patsubst %.c,$(CONFIGURATION_DIRECTORY)/local/%.o,$(LOCAL_SOURCES)) \
    $(patsubst %.c,$(CONFIGURATION_DIRECTORY)/shared/%.o,$(PROJECT_TOOL_SHARED_SOURCES))
BINARY := $(ROOT)/ama-project
COMPILE_COMMAND = $(C_COMPILER) $(CPPFLAGS) -I$(ROOT)/src $(C_COMPILER_FLAGS) $(LTO_FLAGS) -MMD -MP
LINK_COMMAND = $(C_COMPILER) $(C_COMPILER_FLAGS) $(LTO_FLAGS) $(LINKER_FLAGS)
include $(ROOT)/make/configuration.mk
.PHONY: all clean
all: $(BINARY)
$(BINARY): $(OBJECTS) $(CONFIGURATION_DIRECTORY)/link.configuration
>    $(LINK_COMMAND) -o $@ $(OBJECTS)
$(CONFIGURATION_DIRECTORY)/local/%.o: %.c $(CONFIGURATION_DIRECTORY)/compile.configuration Makefile $(ROOT)/make/configuration.mk
>    @mkdir -p $(dir $@)
>    $(COMPILE_COMMAND) -c $< -o $@
$(CONFIGURATION_DIRECTORY)/shared/%.o: $(ROOT)/src/%.c $(CONFIGURATION_DIRECTORY)/compile.configuration Makefile $(ROOT)/make/configuration.mk
>    @mkdir -p $(dir $@)
>    $(COMPILE_COMMAND) -c $< -o $@
-include $(OBJECTS:.o=.d)
clean:
>    $(RM) -r $(CONFIGURATION_DIRECTORY) $(BINARY)
