# @file
# @brief Compile and execute the minimal maintained Ama/native project normally and with target ASan/UBSan.
# Contract: ../../../docs/implementation/library-tests-and-build.md; generated output is never a maintained input.
ROOT := $(abspath ../../..)
TEST_DEFINES := -DTEST_SDK='"$(ROOT)/src"' -DAMA_EXECUTABLE='"$(ROOT)/ama"'
include $(ROOT)/tests/leaf.mk

.PHONY: compiler
compiler:
>    $(MAKE) -C $(ROOT) all

generated/test: | compiler
