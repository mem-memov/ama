ROOT := $(abspath ../../..)
TEST_DEFINES := -DTEST_SDK='"$(ROOT)/src"' -DAMA_EXECUTABLE='"$(ROOT)/ama"'
include $(ROOT)/tests/leaf.mk

.PHONY: compiler
compiler:
>    $(MAKE) -C $(ROOT) all

generated/test: | compiler
