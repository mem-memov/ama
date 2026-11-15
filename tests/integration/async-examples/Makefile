# @file
# @brief Compile and run the public asynchronous Ama and SDK-free native host examples through product commands.
# Contract: ../../../docs/implementation/asynchronous-host.md.
ROOT := $(abspath ../../..)
TEST_DEFINES := -DAMA_EXECUTABLE='"$(ROOT)/ama"' -DTEST_REPOSITORY_ROOT='"$(ROOT)"' -DTEST_SDK='"$(ROOT)/src"' -DTEST_STANDARD_ROOT='"$(ROOT)/stdlib/modules"'
include $(ROOT)/tests/leaf.mk

.PHONY: compiler
compiler:
>    $(MAKE) -C $(ROOT) all

generated/test: | compiler
