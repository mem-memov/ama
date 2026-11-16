# @file
# @brief Run actual source-tool command processes against a complete workspace with an unused importer and opaque drafts/assets.
ROOT := $(abspath ../../../..)
TEST_DEFINES := -DAMA_EXECUTABLE='"$(ROOT)/ama"'
include $(ROOT)/tests/leaf.mk
.PHONY: compiler
compiler:
>    $(MAKE) -C $(ROOT) all
generated/test: | compiler
