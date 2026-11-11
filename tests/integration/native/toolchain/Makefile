ROOT := $(abspath ../../../..)
SOURCES := native/toolchain/symbols.c native/toolchain/listing/record.c native/toolchain/listing/match.c
TEST_DEFINES = -DTEST_ROOT='"$(ROOT)"' -DTEST_REAL_COMPILER='"$(C_COMPILER)"'
include $(ROOT)/tests/leaf.mk
