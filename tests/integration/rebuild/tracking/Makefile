ROOT := $(abspath ../../../..)
TEST_DEFINES = -DTEST_ROOT='"$(ROOT)"' -DTEST_REAL_COMPILER='"$(C_COMPILER)"'
include $(ROOT)/tests/leaf.mk

generated/compiler: project/compiler.c generated/compile.configuration
>    $(C_COMPILER) $(CPPFLAGS) $(C_COMPILER_FLAGS) -o $@ $<

generated/test: | generated/compiler
