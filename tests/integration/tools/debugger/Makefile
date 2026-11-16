# @file
# @brief Compile the shipped debugger client against real emitted assemblies and inspect explicit execution boundaries.
ROOT := $(abspath ../../../..)
C_COMPILER ?= gcc
TEST_DEFINES := -DAMA_EXECUTABLE='"$(ROOT)/ama"' -DTEST_SDK='"$(ROOT)/src"' -DTEST_LAUNCHER='"$(ROOT)/tools/debugger/main.c"' -DTEST_STDLIB='"$(ROOT)/stdlib/modules"' -DTEST_C_COMPILER='"$(C_COMPILER)"'
include $(ROOT)/tests/leaf.mk
.PHONY: compiler
compiler:
>    $(MAKE) -C $(ROOT) all
generated/test: | compiler
