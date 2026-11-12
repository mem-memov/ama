# Exercise overlapping recursive command-manifest checks with a controlled C shell.
ROOT := $(abspath ../../../..)
TEST_DEFINES := -DTEST_ROOT='"$(ROOT)"'
include $(ROOT)/tests/leaf.mk

generated/shell: project/shell.c generated/compile.configuration
>    $(C_COMPILER) $(CPPFLAGS) $(C_COMPILER_FLAGS) $(LTO_FLAGS) $(LINKER_FLAGS) -o $@ $<

generated/test: | generated/shell
