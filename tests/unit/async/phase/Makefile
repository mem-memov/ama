# @file
# @brief Check phase selection and standalone native write-view publication.
# Contract: ../../../../docs/implementation/task-phases.md.
ROOT := $(abspath ../../../..)
C_COMPILER ?= gcc
TEST_DEFINES := -DTEST_ROOT='"$(ROOT)"' -DTEST_C_COMPILER='"$(C_COMPILER)"'
ifneq ($(findstring -fsanitize=,$(C_COMPILER_FLAGS)),)
TEST_DEFINES += -DTEST_SANITIZERS
endif
include $(ROOT)/tests/leaf.mk

.PHONY: compiler
compiler:
>    $(MAKE) -C $(ROOT) all

generated/test: | compiler
