# @file
# @brief Verify actual CLI native-template refusal before companion/header publication.
# Contract: ../../../docs/implementation/native-static-admission.md.
ROOT := $(abspath ../../..)
TEST_DEFINES := -DAMA_EXECUTABLE='"$(ROOT)/ama"' -DTEST_SDK='"$(ROOT)/src"'
include $(ROOT)/tests/leaf.mk

.PHONY: compiler
compiler:
>    $(MAKE) -C $(ROOT) all

generated/test: | compiler
