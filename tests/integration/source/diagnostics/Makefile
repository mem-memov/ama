ROOT := $(abspath ../../../..)
include $(ROOT)/make/sources.mk
SOURCES := $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/source)) \
    $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/diagnostics))
TEST_DEFINES = -DTEST_REAL_COMPILER='"$(C_COMPILER)"'
include $(ROOT)/tests/leaf.mk
