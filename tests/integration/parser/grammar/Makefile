ROOT := $(abspath ../../../..)
include $(ROOT)/make/sources.mk
SOURCES := $(foreach component,source diagnostics lexer ast parser,\
    $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/$(component))))
TEST_DEFINES = -DTEST_REAL_COMPILER='"$(C_COMPILER)"' -DTEST_ROOT='"$(ROOT)"'
include $(ROOT)/tests/leaf.mk
