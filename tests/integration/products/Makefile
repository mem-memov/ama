# @file
# @brief Verify repeatable product generation and changed-input behavior through real compiler processes.
ROOT := $(abspath ../../..)
TEST_DEFINES := -DTEST_ROOT='"$(ROOT)"'
include $(ROOT)/tests/leaf.mk
.PHONY: compiler
compiler:
>    $(MAKE) -C $(ROOT) all
generated/test: | compiler
