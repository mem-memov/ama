ROOT := $(abspath ../../../..)
include $(ROOT)/tests/unit/native/scaffold/sources.mk
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/codegen))
TEST_DEFINES := -DTEST_SDK=\"$(ROOT)/src\"
include $(ROOT)/tests/leaf.mk
